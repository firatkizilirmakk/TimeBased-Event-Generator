#include <iostream>
#include <chrono>
#include <functional>

using CLOCK = std::chrono::high_resolution_clock;
using TTimerCallback = std::function<void()>;
using Millisecs = std::chrono::milliseconds;
using Timepoint = CLOCK::time_point;
using TPredicate = std::function<bool()>;

/**
 * Used to model an event by containing
 * necessary stuff as a class member
 * 
 */
class Event{
  public:
    /*  maximum timepoint given to handle finding minimum */
    Event(){tp = Timepoint::max();}    
    Event(Timepoint timepoint, TTimerCallback callback) : tp(timepoint),cb(callback){}
    Event(Millisecs milisec, TTimerCallback callback) : ms(milisec),cb(callback) {}
    Event(Timepoint timepoint, Millisecs milisec, TTimerCallback callback) : tp(timepoint), ms(milisec), cb(callback){}
    Event(TPredicate predicate, Millisecs milisec, TTimerCallback callback) : pred(predicate), ms(milisec), cb(callback){}

    Timepoint& getTimePoint() { return tp;}
    void setTimePoint(Timepoint tp) {this->tp = tp;}

    TTimerCallback& getCallback() { return cb;}
    void setCallback(TTimerCallback cb) {this->cb = cb;}

    Millisecs& getMillisecs() {return ms;}
    void setMillisecs(Millisecs ms) {this->ms = ms;}

    TPredicate& getPredicate() {return pred;}
    void setPredicate(TPredicate tpred) {this->pred = tpred;}

    int getEventType(){return type;}
    void setEventType(int type){this->type = type;}

    bool operator == (const Event& other){
        return (tp == other.tp && ms == other.ms);
    }

  private:
    Timepoint tp;
    TTimerCallback cb;
    Millisecs ms;
    TPredicate pred;

    /* Event type stating whether called for first type,second etc. */
    int type;
};