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
	closesocket(s);	// Close all sockets
	{
		lock_guard<mutex> sets(mEX_Sets);		// Lock mutex for the three fd sets
		for (size_t i = 0; i < ConnectionSet.fd_count; ++i) { closesocket(ConnectionSet.fd_array[i]); }
		for (size_t i = 0; i < ErrorSet.fd_count; ++i) { closesocket(ErrorSet.fd_array[i]); }
	}
	// Wait to cue thread exit until just before joining. If not, thread t may finish first and prematurely terminate the program upon completion.
	killConnection.store(TRUE, memory_order_release);	// Set "kill connection" flag to TRUE. This will cue the loop polling the sockets to exit.
	if (t.joinable()) { t.join(); }		// Join the connection thread before destruction. Check first that it's joinable.

	WSACleanup();	// Cleanup any lingering WinSock data
}

void Setup_Window_Layout() {
	//Create all needed windows and store their handles for later use.
	interfaceWindows.push_back(CreateWindow(TEXT("edit"), TEXT(""), WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, 0, 0, 500, 500, hwnd, (HMENU)ID_OUTPUT_WINDOW, hInst, NULL));
	interfaceWindows.push_back(CreateWindow(TEXT("edit"), TEXT(""), WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 0, 500, 500, 75, hwnd, (HMENU)ID_INPUT_WINDOW, hInst, NULL));
	interfaceWindows.push_back(CreateWindow(TEXT("button"), TEXT("SEND"), WS_CHILD | WS_BORDER, 500, 500, 150, 75, hwnd, (HMENU)IDC_SEND_MESSAGE, hInst, NULL));

	connectionWindows.push_back(CreateWindow(TEXT("button"), TEXT("CONNECT"), WS_CHILD | WS_VISIBLE | WS_BORDER, 200, 0, 100, 25, hwnd, (HMENU)IDC_CONNECT, hInst, NULL));
	connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("127"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 0, 0, 50, 25, hwnd, (HMENU)ID_IPv4_pt1, hInst, NULL));
	connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("0"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 50, 0, 50, 25, hwnd, (HMENU)ID_IPv4_pt2, hInst, NULL));
	connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("0"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 100, 0, 50, 25, hwnd, (HMENU)ID_IPv4_pt3, hInst, NULL));
	connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("1"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 150, 0, 50, 25, hwnd, (HMENU)ID_IPv4_pt4, hInst, NULL));
	connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("7777"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 0, 25, 50, 25, hwnd, (HMENU)ID_PORT_NUMBER, hInst, NULL));
	connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("USER NAME"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 100, 25, 100, 25, hwnd, (HMENU)ID_USERNAME, hInst, NULL));

	//Intercepts messages to input box for additional processing. Stores default procedure in EditHandler to return messages when finished.
	//This is necessary to allow sending messages with the return key.
	EditHandler = (WNDPROC)SetWindowLong(GetDlgItem(hwnd, ID_INPUT_WINDOW), GWL_WNDPROC, (LONG)Input_Box_Subclass);
	//Also subclass IP input windows for additional functionality and more intuitive UI. See Edit_handler.cpp for details.
	for (auto i = connectionWindows.begin() + 1; i != connectionWindows.end(); ++i) { SetWindowLong(*i, GWL_WNDPROC, (LONG)Connection_Box_Subclass); }
}

//bool SOCKETMANAGER::Connect_to_Server(string IPv4_1, string IPv4_2, string IPv4_3, string IPv4_4, string portNumber, string userName) {
bool SOCKETMANAGER::Connect_to_Server() {
	//Read IP address and port number from input boxes
	string IPv4_1 = wstring_to_string(Edit_Box_to_Wstring(GetDlgItem(hwnd, ID_IPv4_pt1)));
	string IPv4_2 = wstring_to_string(Edit_Box_to_Wstring(GetDlgItem(hwnd, ID_IPv4_pt2)));
	string IPv4_3 = wstring_to_string(Edit_Box_to_Wstring(GetDlgItem(hwnd, ID_IPv4_pt3)));
	string IPv4_4 = wstring_to_string(Edit_Box_to_Wstring(GetDlgItem(hwnd, ID_IPv4_pt4)));
	string portNumber = wstring_to_string(Edit_Box_to_Wstring(GetDlgItem(hwnd, ID_PORT_NUMBER)));
	string userName = wstring_to_string(Edit_Box_to_Wstring(GetDlgItem(hwnd, ID_USERNAME)));

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

	for (auto i : interfaceWindows) { SetWindowPos(i, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE); }	//Show all interface windows
	for (auto i : connectionWindows) { SetWindowPos(i, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE); }	//Hide all connection windows

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
			Sleep(sleepTime);			//Denoted in miliseconds.
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

		auto message = string{ };
		auto bytesRead = ReadSocketMessage(s, message);
		if (bytesRead > 0 && message.size() > 0) {
			inputQ.push(message);
			PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_INCOMING_MESSAGE, NULL), NULL);	// Notify main window of available messages.
		}
		else if (bytesRead < 0) {
			closesocket(s);
			killConnection.store(TRUE, memory_order_release);
			MessageBox(hwnd, L"Connection to server lost.", L"ERROR", MB_OK);
			continue;
		}
		else { continue; }
	}

	return;
}

int ReadSocketMessage(SOCKET s, string& msg) {
	constexpr auto buffSize{ 1024 };							// Define buffer size
	char input_C_string[buffSize];								// Buffer to hold input
	int bytes{ 0 };												// Initialize var to track bytes received (or error)
	msg.clear();												// Clear return string
	
	while (true) {
		memset(input_C_string, '\0', buffSize);					// Fill array with null values ensuring null-terminator
		auto len = recv(s, input_C_string, buffSize - 1, 0);	// Receive message
		if (len < 1) {
			closesocket(s);
			bytes = len;			// Error! Close socket and return -1 to flag error state
			break;
		}
		else {
			bytes += len;
			msg.append(input_C_string);						// Append input to return string
			if (input_C_string[buffSize - 2] == '\0') { break; }	// Message completed since it didn't fill the buffer
		}
	}
	return bytes;
}

void SOCKETMANAGER::Queue_Message() {
	HWND h = GetDlgItem(hwnd, ID_INPUT_WINDOW);
	auto temp_out = Edit_Box_to_Wstring(h);
	auto out = wstring_to_string(temp_out);
	Q.push(out);
	SendMessage(h, WM_COMMAND, MAKEWPARAM(IDC_RESET_BOX, NULL), NULL); /*SetWindowText(h, L"");*/
}

void Receive_Message() {
	wstring out = string_to_wstring(*inputQ.load_and_pop());
	out.append(L"\n");		//Adds new-line separation after each message.
	HWND h = GetDlgItem(hwnd, ID_OUTPUT_WINDOW);

	//Places new messages at the end of the text in the output window.
	int sel = GetWindowTextLength(h);
	SendMessage(h, EM_SETSEL, (WPARAM)sel, (LPARAM)sel);
	SendMessage(h, EM_REPLACESEL, 0, (LPARAM)((LPWSTR)out.c_str()));
}