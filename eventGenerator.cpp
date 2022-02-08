#include "eventGenerator.hpp"

TimerEventGenerator::TimerEventGenerator(){
  //use move assignment operator 
  th = std::thread{&TimerEventGenerator::eventGeneratorThread,this};
}

/**
 * Join the thread to wait its termination
 */
TimerEventGenerator::~TimerEventGenerator(){
  th.join();
}

/**
 * Register an aperiodic event
 * 
 * @param tp Timepoint to call the callback
 * @param cb Callback to be called
 */
void TimerEventGenerator::registerTimer(const Timepoint& tp, const TTimerCallback& cb){
  Event event = Event(tp,cb);
  event.setEventType(0);
  {
    std::lock_guard<std::mutex> lk(queueMutex);
    eventQueue.push(event);
  }
  cv.notify_all();
}

/**
 * Register a periodic event
 * 
 * @param period Period of the event
 * @param cb Callback to be called
 */
void TimerEventGenerator::registerTimer(const Millisecs& period, const TTimerCallback& cb){
  Event event = Event(period,cb);
  Timepoint tp = CLOCK::now();
  
  // set timepoint now to call immediately
  event.setTimePoint(tp);
  event.setEventType(1);
  {
    std::lock_guard<std::mutex> lk(queueMutex);
    eventQueue.push(event);
  }
  cv.notify_all();  
}

/**
 * Register a periodic event
 * 
 * @param tp Timepoint to start calling
 * @param period Period of the event
 * @param cb Callback to be called
 */
void TimerEventGenerator::registerTimer(const Timepoint& tp, const Millisecs& period, const TTimerCallback& cb){
  Event event = Event(tp, period, cb);
  
  // set timepoint tp to start calling at tp
  event.setTimePoint(tp);
  event.setEventType(2);
  {
    std::lock_guard<std::mutex> lk(queueMutex);
    eventQueue.push(event);
  }
  cv.notify_all();
}

/**
 * Register a periodic event
 * 
 * @param pred Predicate function to state callabality of callback
 * @param period Period of the event
 * @param cb Callback to be called
 */
void TimerEventGenerator::registerTimer(const TPredicate& pred, const Millisecs& period, const TTimerCallback& cb){
  Event event = Event(pred,period,cb);
  Timepoint tp = CLOCK::now();
  
  // set timepoint now to call immediately
  event.setTimePoint(tp);
  event.setEventType(3);
  {
    std::lock_guard<std::mutex> lk(queueMutex);
    eventQueue.push(event);
  }
  cv.notify_all();
}

/**
 * Event Generator Thread to handle 
 * generating events at the right time
 */
void TimerEventGenerator::eventGeneratorThread(){
  while(1){
    std::unique_lock<std::mutex> lk(queueMutex);
    /* wait if there is no event registered,no event to be processed  */
    cv.wait(lk,[this]{     
        return eventQueue.size() != 0 || periodicEvents.size() != 0 || aperiodicEvents.size() != 0;
    });
    lk.unlock();
    
    Event& nextEvent = getNextEvent();
    Timepoint tp = nextEvent.getTimePoint();

    /* lock the mutex to access queue */
    lk.lock();
    int oldSize = eventQueue.size();
    /* wait until there is a new event registered  */
    /* or timepoint exceeds */
    cv.wait_until(lk,tp);
    int newSize = eventQueue.size();
    lk.unlock();

    /* no new element, woke for tp */
    if(oldSize == newSize){
      bool callable = true;
      int eventType = nextEvent.getEventType();

      /* check predicate situation */
      if(eventType == 3){
        TPredicate pred = nextEvent.getPredicate();
        if(!pred())
          callable = false;
      }

      /* call the callback if it is callable, in case of 3rd event type */
      if(callable){
        Timepoint now = CLOCK::now();
        if(now > tp + Millisecs(5) || now < tp - Millisecs(5)){
          std::cout << "\nTimeout !";
        }
        TTimerCallback cb = nextEvent.getCallback();
        cb();
      }
      
      /* aperiodic event must be removed from list after running */
      /* or periodic event with false predicate */
      if(eventType == 0 || (eventType == 3 && !callable))
        removeNextEvent(nextEvent);
      if(eventType != 0){
        /* periodic events' timepoint must be set to next period */
        Timepoint nextTp = nextEvent.getTimePoint() + nextEvent.getMillisecs();
        nextEvent.setTimePoint(nextTp);
      }
    }
  }
}

/**
 * Puts the just pushed events into 
 * proper lists.Finds an event with
 * earlieast timepoint and returns it
 * @return Event with earlist timepoint
 */
Event& TimerEventGenerator::getNextEvent(){
  /* get the last pushed events */
  {
    Event e;
    std::lock_guard<std::mutex> guard(queueMutex);
    for(int i = 0 ; i < eventQueue.size() ; ++i){
      e = eventQueue.front();
      eventQueue.pop();

      /* put the event into proper list */
      int type = e.getEventType();
      if(type == 0){
        aperiodicEvents.push_back(e);
      }else{
        periodicEvents.push_back(e);
      }
    }
  }

  Event min;
  int minIndexAperiodic = -1;
  int minIndexPeriodic = -1;
  
  /* find event wrt earliest timepoint */
  int size = aperiodicEvents.size();
  for(int i = 0 ; i < size ; ++i){
    Event& e = aperiodicEvents[i];

    if(e.getTimePoint()  < min.getTimePoint()){
      min = e;
      minIndexAperiodic = i;
    }
  }

  size = periodicEvents.size();
  for(int i = 0 ; i < size ; ++i){
    Event& e = periodicEvents[i];

    if(e.getTimePoint()  < min.getTimePoint()){
      min = e;
      minIndexPeriodic = i;
    }
  }
  
  /* return found event */
  if(minIndexAperiodic == -1 && minIndexPeriodic != -1)
    return periodicEvents.at(minIndexPeriodic);
  else if(minIndexPeriodic == -1 && minIndexAperiodic != -1 )
    return aperiodicEvents.at(minIndexAperiodic);
  else{
    if(aperiodicEvents[minIndexAperiodic].getTimePoint() < periodicEvents[minIndexPeriodic].getTimePoint()){
      return aperiodicEvents.at(minIndexAperiodic);
    }else{
      return periodicEvents.at(minIndexPeriodic);
    }
  }
}

/**
 * Remove given element from one of the
 * event lists using overloaded 
 * equality operator
 * @param Event Event to be removed
 */
void TimerEventGenerator::removeNextEvent(Event event){
  int size = aperiodicEvents.size();
  Timepoint eventTp = event.getTimePoint();
  TTimerCallback callBack = event.getCallback();

  if(event.getEventType() == 0){
    auto itr = aperiodicEvents.begin();
    while(itr != aperiodicEvents.end()){
      Event e = *itr;

      if(e == event){
        aperiodicEvents.erase(itr);
        break;
      }
      itr++;
    }
  }else{
    auto itr = periodicEvents.begin();
    while(itr != periodicEvents.end()){
      Event e = *itr;

      if(e == event){
        periodicEvents.erase(itr);
        break;
      }
      itr++;
    }
  }
}