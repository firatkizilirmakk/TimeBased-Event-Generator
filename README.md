## Time Based Event Generator

This repository provides time based event generation functions written in C++, with 11 standards. It comes with a very simple API:

* Create an object of *TimerEventGenerator* class
* Register a callback function (event) using  one of the four different overloaded *registerTimer* functions, which are explained below.

That is all. The *TimerEventGenerator* object implicitly creates a thread to monitor the time, and to generate the event at necessary times.

## Usage
Timer interface provides four overloaded functions, **registerTimer** function with four different signatures:

1. aperidoc event that is generated once at a specified time.
2. periodic event that is generated:
	1. once it is created, and then periodically
	2. at a speficied time, and then peridically
	3. within periods, with respect to a boolean predicate

#### Example
First of first, instantiate a *TimerEventGenerator* object. Then, create a time point when you want to generate an event. Example below: 5 seconds from now on.

	TimerEventGenerator teg;
	Timepoint tp = CLOCK::now() + Millisecs(5000);

Register an aperiodic event to be generated at the timepoint specified above:

	  teg.registerTimer(tp, []{ 
	    std::cout << "Callback function, aperiodic event at " << CLOCK::to_time_t(CLOCK::now()) << std::endl;
	  });

When the specified time is reached, the callback function which prints *"Callback function, aperiodic event at ..."* is called. Since this function only takes a timepoint argument, it is aperiodic thus called once.

Below given other example usages:

* In order to generate periodic events within specified periods:

		  teg.registerTimer(Millisecs(2000), []{
		    std::cout << "Callback2, periodic event at " << CLOCK::to_time_t(CLOCK::now()) << std::endl;
		  });

	This callback function is executed as soon as it is registered, and then called within 2 seconds periods.

* To generate periodic events within specified periods, but starting at a later timepoint:

		  teg.registerTimer(tp, Millisecs(1000), []{
		    std::cout << "Callback3, periodic event with timepoint at " << CLOCK::to_time_t(CLOCK::now()) << std::endl;
		  });

	The callback function is called first after 5 seconds (tp defined above), and then executed periodically within 1 seconds.

* To generate periodic events while considering a boolean predicate:

		  teg.registerTimer([]{
		    if(CLOCK::to_time_t(CLOCK::now()) % 2 == 0)
		      return true;
		    return false;
		  }, Millisecs(3000), []{
		    std::cout << "Callback4, periodic with predicate at " << CLOCK::to_time_t(CLOCK::now()) << std::endl;
		  });

	This callback function is called within 3 seconds, if the predicate function also yields true. Here, the predicate controls the time; whether it is even or not. When it is intended to execute the callback function , the predicate function is checked, and if it produces true then the callback function runs.
