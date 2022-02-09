#include <iostream>
#include <chrono>

#include "eventGenerator.hpp"

int main(int argc,char* argv[]){
  TimerEventGenerator teg;
  Timepoint tp = CLOCK::now() + Millisecs(5000);

  printf("Current time: %ld\n",CLOCK::to_time_t(CLOCK::now()));

  teg.registerTimer(tp, []{    
    std::cout << "\nCallback, aperiodic event at " << CLOCK::to_time_t(CLOCK::now()) << std::endl;
  });

  teg.registerTimer(Millisecs(2000), []{
    std::cout << "\nCallback2, periodic event at " << CLOCK::to_time_t(CLOCK::now()) << std::endl;
  });

  teg.registerTimer(tp, Millisecs(1000), []{
    std::cout << "\nCallback3, periodic event with timepoint at " << CLOCK::to_time_t(CLOCK::now()) << std::endl;
  });

  teg.registerTimer([]{
    if(CLOCK::to_time_t(CLOCK::now()) % 2 == 0)
      return true;
    return false;
  }, Millisecs(3000), []{
    std::cout << "\nCallback4, periodic with predicate at " << CLOCK::to_time_t(CLOCK::now()) << std::endl;
  });

  return 0;
}