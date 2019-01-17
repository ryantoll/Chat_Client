/*////////////////////////////////////////////////////////////////////////////////////////////////////
//Chat_Client
//Ryan Toll 2018
//
//This is primary implementation of the network interface commands.
//The function names should be rather self-explanitory.
//The header file lays out the purpose of each component.
////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "stdafx.h"

SOCKETMANAGER::SOCKETMANAGER() {
	//Start up WinSock2
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		MessageBox(hwnd, L"WinSock2 failed to initialize.", L"ERROR", MB_OK);
	}

	//Ensure sets are empty before use.
	FD_ZERO(&ConnectionSet); FD_ZERO(&ErrorSet);

	//Set up connection parameters used for all socket connections.
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		//IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	//Stream connection type
	hints.ai_protocol = IPPROTO_TCP;	//TCP/IP protocol for connection
}

SOCKETMANAGER::~SOCKETMANAGER() {
	//Close all sockets
	closesocket(s);

	unique_lock<mutex> sets(mEX_Sets);		//Lock mutex for the three fd sets
	for (size_t i = 0; i < ConnectionSet.fd_count; ++i) { closesocket(ConnectionSet.fd_array[i]); }
	for (size_t i = 0; i < ErrorSet.fd_count; ++i) { closesocket(ErrorSet.fd_array[i]); }
	sets.unlock();							//Unlock mutex for the three fd sets once done.
											//Note, unlocking may be overkill here since the SOCKETMANAGER destructor should only be run when everything else is shutting down.
											//unique_lock<> or lock_guard<> both release the mutex automatically in their in their own destructor when the local variable expires.

											//Wait to cue thread exit until just before joining. If not, thread t may finish first and prematurely terminate the program upon completion.
	killConnection.store(TRUE, memory_order_release);		//Set "kill connection" flag to TRUE. This will cue the loop polling the sockets to exit.
	if (t.joinable()) { t.join(); }		//Join the connection thread before destruction. Check first that it's joinable.

										//Cleanup any lingering WinSock data
	WSACleanup();
}

bool SOCKETMANAGER::Connect_to_Server(string IPv4_1, string IPv4_2, string IPv4_3, string IPv4_4, string portNumber, string userName) {
	SOCKET newSocket = INVALID_SOCKET;
	int status = 0;
	addrinfo* res;
	string IPv4_address = IPv4_1 + "." + IPv4_2 + "." + IPv4_3 + "." + IPv4_4;

	//Set up address structure based off of input parameters. Store results in pointer: res.
	status = getaddrinfo(NULL, portNumber.c_str(), &hints, &res);

	//Iterate through all addresses in set until one works.
	for (addrinfo* i = res; i != NULL; i = i->ai_next) {
		newSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
		if (newSocket == INVALID_SOCKET) { closesocket(newSocket); continue; }


		status = connect(newSocket, i->ai_addr, i->ai_addrlen);	
		if (status > -1) { break; }		//Exit loop upon success.
	}
	if (status == -1) { closesocket(newSocket); MessageBox(hwnd, L"Socket connection failed.", L"ERROR", MB_OK); return FALSE; }

	//Set up socket for keep-alive packets to maintain connection.
	bool bSockOpt = TRUE;
	int bOptLen = sizeof(bool);
	status = setsockopt(newSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&bSockOpt, bOptLen);
	if (status == -1) { closesocket(newSocket); MessageBox(hwnd, L"Socket connection failed.", L"ERROR", MB_OK); return FALSE; }

	freeaddrinfo(res);

	s = newSocket;			//Open connection to server.
	FD_SET(s, &ConnectionSet);		//Add socket to set.
	killConnection.store(FALSE, memory_order_release);		//Set "kill connection" flag to FALSE.
															//Acquire/release semantics are used to enforce adequate memory ordering at minimal cost.

	send(s, userName.c_str(), userName.size(), NULL);		//Upon successful connection, send in the username so the server can register it.

	t = thread(&SOCKETMANAGER::PollPort, this);		//Create a new thread constructed with the PollPorts function called on this object. Assign this new thread to variable t.
	MessageBox(hwnd, L"Connected", L"INFO", MB_OK);

	return TRUE;
}

//Note that the present form of this was adapted from prior work and therefore retains some redundant generic elements.
//Some of this could be simplified given that there is only one connection, though the server will likely need these elements.
void SOCKETMANAGER::PollPort() {
	//Run indefinitely until cued to stop.
	while (!killConnection.load(memory_order_acquire)) {
		unique_lock<mutex> sets(mEX_Sets, defer_lock);		//Associates variable "sets" with mutex mEX_Sets while leaving the mutex unlocked.

																			//The function gets its own copy of the fd sets to prevent data collision/contention.
																			//This is remade each pass with the most current data.
		sets.lock();		//Lock fd sets
			fd_set readFDS = ConnectionSet, exceptFDS = ErrorSet;		//Read from all vaild connections.
		sets.unlock();		//Unlock fd sets

		timeval tv;
		tv.tv_sec = 1;
		int ready = select(NULL, &readFDS, NULL, &exceptFDS, &tv);
		//Supposedly the select statement blocks up to the length of timeval "tv", however my experience has been the opposite.
		//Instead, it seems to return immediately as having failed if no connection is ready. Hence, I loop it until it succeeds.
		//I use the Sleep() command to manually enforce a sleep time before repeating the select() command since it's not actually blocking as expected.
		//I wouldn't want to burn excessive CPU cycles constantly checking and re-checking the socket as fast as possible.

		//Idle until there are messages to receive. Output is still sent in idle loop.
		while (ready == 0 && !killConnection.load(memory_order_acquire)) {
			Send_Messages();	//Check on each loop for output to send.
			Sleep(25);			//Denoted in miliseconds.
			sets.lock();		//Lock fd sets
				readFDS = ConnectionSet;
				exceptFDS = ErrorSet;
			sets.unlock();		//Unlock fd sets
			ready = select(NULL, &readFDS, NULL, &exceptFDS, &tv);
		}

		if (killConnection) { return; }

		if (ready == -1) {
			int x = WSAGetLastError();
			MessageBox(hwnd, string_to_wstring(gai_strerrorA(x)).c_str(), L"ERROR", MB_OK);
		}


		sets.lock();		//Lock fd sets
		fd_set writeFDS = ConnectionSet;	//Write to all valid connections.
		sets.unlock();		//Unlock fd sets

		for (unsigned int i = 0; i < readFDS.fd_count; ++i) {
			unique_ptr<char[]> input_C_string(new char[1024]);		//Dynamically allocate new array to store conversion output.
			int bytesRead = recv(readFDS.fd_array[i], input_C_string.get(), 1023, 0);

			//If connection is closed greacefully, recv() succeeds with bytesRead == 0. If it is closed in an ungraceful manner, recv() returns -1.
			//Either way, the connection should shutdown. Failure to do so results in infinately reading emptiness into the queue to post for output.
			if (bytesRead < 1) {
				closesocket(readFDS.fd_array[i]);
				s = INVALID_SOCKET;
				sets.lock();
				FD_CLR(readFDS.fd_array[i], &ConnectionSet);
				if (ConnectionSet.fd_count == 0) { killConnection.store(TRUE, memory_order_release); }	//If there are no connection left, flag shutdown.
				sets.unlock();
				MessageBox(hwnd, L"Connection to server lost.", L"ERROR", MB_OK);
				continue;
			}
			else if (bytesRead == 1) {
				continue;
			}

			//string message = input_C_string.get();
			string message(input_C_string.get());
			inputQ.push(message);
			PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_INCOMING_MESSAGE, NULL), NULL);	//Notify main window of available messages.
		}

		
		//This doesn't seem to get triggered as written since the error actually pops up in the recv() command.
		//Error handling has been incorporated into the recv() loop.
		//The loop below illustrates how I would otherwise clear out sockets that have fallen into an error state.

		//Close any sockets that return error codes.
//		for (unsigned int i = 0; i < exceptFDS.fd_count; ++i) {
//			SOCKET s = exceptFDS.fd_array[i];
//			closesocket(s);
//			sets.lock();	//Lock fd sets
//			FD_CLR(s, &ConnectionSet);
//			FD_CLR(s, &ErrorSet);
//			sets.unlock();	//Unlock fd sets
//
//			//Send error message.
//			wstring out = L"Connection to ";
//			out += L"_____";		//Placeholder. Connections are unnamed at this point.
//			out += L" was lost.";
//			MessageBox(hwnd, out.c_str(), L"ERROR", MB_OK);
//		}
	}

	return;
}