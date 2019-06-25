/*////////////////////////////////////////////////////////////////////////////////////////////////////
//Chat_Server
//Ryan Toll 2018
//
//This is a lock-based, threadsafe queue for sending data between threads.
//Each queue is envisioned to have a one-way flow of information.
//A second queue can be established for two-way communicaiton.
//The port-polling thread will check for data to send on each loop; any data indicates work to do.
//The GUI can't be readily looped in the same way without blocking user input.
//(PeekMessage function could be used to check for user input, but is not my prefered solution.)
//Instead, the GUI thread will be notified of incoming data with the (threasafe) PostMessage function.
//The GUI thread will then process all data in the queue before returning to the Window's message loop.
//This should give a responsive user experience as the blocking operations are brief and as needed.
//
//A generic queue could be made by replacing the string parameter with a generic template parameter.
//Such an excercise is beyond the scope of this project.
//
//This queue should provide ample performance since locks are held only very briefly.
//Only adding/copying a pointer and testing emptiness will happen while locked.
//Any expensive operation such as allocating/initializing data is done outside of locks.
//This allows for ample concurrency in situations of low to medium contention.
////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef THREADSAFE_QUEUE
#define THREADSAFE_QUEUE

#include "stdafx.h"

class QUEUE_THREADSAFE {
public:
	//Contructor and Destructor are implicitly default constructed.
	//QUEUE_THREADSAFE(){};
	//~QUEUE_THREADSAFE(){};

	bool empty() { lock_guard<mutex> lock(m); return Q.empty(); }	//Lock queue and 

	//Add shared_ptr to queue
	void push(unique_ptr<string> s) {
		lock_guard<mutex> lock(m);	//Unlocks mutex automatically when the variable goes out of scope.
		Q.push(std::move(s));	//Add pointer to queue
	}

	//Add by value instead of pointer
	void push(const string& add) {
		auto s = make_unique<string>(add);	//Expensive operation done before locking occurs.
		lock_guard<mutex> lock(m);	//Unlocks mutex automatically when the variable goes out of scope.
		Q.push(std::move(s));	//Add pointer to queue
	}

	//Return frontmost pointer in queue and remove from queue.
	unique_ptr<string> load_and_pop() {
		unique_ptr<string> s;			//Create unique pointer
		lock_guard<mutex> lock(m);		//Unlocks mutex automatically when the variable goes out of scope.
		if (Q.empty()) { return s; }	//Return nullptr if queue is empty.
		s = std::move(Q.front());					
		Q.pop();						//Pop only after successful reading.
		return s;
	}

	//Read frontmost string into user-provided input string and remove from queue.
	bool load_and_pop(string& s) {
		unique_lock<mutex> lock(m);				//Unlocks mutex automatically when the variable goes out of scope. Provides unlock functionality.
			if (Q.empty()) { return FALSE; }	//Return nullptr if queue is empty.
			auto p = std::move(Q.front());
			Q.pop();							//Pop only after successful reading.
		lock.unlock();

		s = *p;		//Dereference pointer outside of lock in case the assignment is time-consuming.
		return TRUE;
	}

private:
	queue<unique_ptr<string>> Q;	//The queue holds unique pointers
	mutex m;
};

#endif // !THREADSAFE_QUEUE

