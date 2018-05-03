/*////////////////////////////////////////////////////////////////////////////////////////////////////
//Chat_Client
//Ryan Toll 2018
//
//Class for handling network communication and related functions.
//There should be only one such object, which is created at startup under the WM_CREATE case.
//This, however, is not presently enforced by the class structure.
//
//Note that the destructor is run as the program exits, wherein I ensure proper cleanup is done.
//Unhandled exceptions may cause the program to crash w/o running the destructor.
//Conceivably I could encapsulate the relvant portions with a try block.
//I would then shut down the connection gracefully in the catch block to always ensure proper shutdown.
////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef SOCKET_MANAGER
#define SOCKET_MANAGER

#include "stdafx.h"

class SOCKETMANAGER {
public:
	SOCKETMANAGER();		//Set up WinSock2 with application-specific parameters
	~SOCKETMANAGER();		//Close out and clean up WinSock2 and associated data

	//The name says it all. Connection parameters are taken from user input.
	//The username is sent to the server upon establishing a successful connection.
	bool Connect_to_Server(string IPv4_1, string IPv4_2, string IPv4_3, string IPv4_4, string portNumber, string userName);

	//Queue message to be sent to server.
	void Push(string out) { Q.push(out); }

private:
	atomic_bool killConnection = TRUE;	//Cue shutdown for network communication thread.
	SOCKET s;							//Socket through which connection is held.
	addrinfo hints;						//Parameters for WinSock2 connection.
	fd_set ConnectionSet, ErrorSet;		//May be redundant for client-side application with a single connection.
	mutex mEX_Sets;						//May be redundant for client-side application with a single connection.
	thread t;							//Variable that "holds" or associates with the network communication thread.
	QUEUE_THREADSAFE Q;					//A custom threadsafe queue that will receive outgoing messages.

	void PollPort();					//Function to handle network communication. Loops indefinitely until cued to shutdown or connection is lost.
										//The work of this function is separated off into a separate thread.
										//Separate threads are used to avoid UI interruptions and to conceptually separate distinct tasks.

	//Send messages to server until the queue is empty.
	void Send_Messages() { while (!Q.empty()) { string out = *Q.load_and_pop();  send(s, out.c_str(), out.length(), 0); } }
};

#endif // !SOCKET_MANAGER