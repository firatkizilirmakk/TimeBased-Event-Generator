#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "itimer.hpp"
#include "event.hpp"

/**
 * Timer Event Generator Concrete Class
 */
class TimerEventGenerator : public ITimer{
  public:

    TimerEventGenerator();
    
    /**
    * Join the thread to wait its termination
    */
    ~TimerEventGenerator();
    
    /**
     * Register an aperiodic event
     * 
     * @param tp Timepoint to call the callback
     * @param cb Callback to be called
     */
    void registerTimer(const Timepoint& tp, const TTimerCallback& cb);

    /**
     * Register a periodic event
     * 
     * @param period Period of the event
     * @param cb Callback to be called
     */
    void registerTimer(const Millisecs& period, const TTimerCallback& cb);

    /**
     * Register a periodic event
     * 
     * @param tp Timepoint to start calling
     * @param period Period of the event
     * @param cb Callback to be called
     */
    void registerTimer(const Timepoint& tp, const Millisecs& period, const TTimerCallback& cb);

    /**
     * Register a periodic event
     * 
     * @param pred Predicate function to state callabality of callback
     * @param period Period of the event
     * @param cb Callback to be called
     */
    void registerTimer(const TPredicate& pred, const Millisecs& period, const TTimerCallback& cb);

    private:
    /**
     * Puts the just pushed events into
     * proper lists.Finds an event with
     * earliest timepoint and returns it
     * @return Event with earliest timepoint
     */
      Event& getNextEvent();

    /**
     * Remove given element from one of the
     * event lists using overloaded 
     * equality operator
     * @param Event Event to be removed
     */
      void removeNextEvent(Event event);

      /**
       * Event Generator Thread to handle 
       * generating events at the right time
       */
      void eventGeneratorThread();

      /* data structures used to hold events */
      std::vector<Event> periodicEvents;
      std::vector<Event> aperiodicEvents;

      /* shared queue storing registered events */
      std::queue<Event> eventQueue;

      /* synchronization primivites  */
      std::condition_variable cv;
      std::mutex cv_m;
      std::mutex queueMutex;

      std::thread th;
};