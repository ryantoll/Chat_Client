/*////////////////////////////////////////////////////////////////////////////////////////////////////
//Chat_Client
//Ryan Toll 2018
//
//Contact me at:
//[last name] (dot) [first name] (at) G M A I L (dot) COM
//
//This program is the client side of an instant messenger application.
//It is designed to be paired with the corresponding server application on a private network.
//The pair is a portfolio piece meant to demonstrate competence with C++ and the Windows API.
//
//Other topics:
//Socket communication with BSD/Winsocket
//Multithreading
//UI/UX
//Class design & encapsulation
//
//Be sure to review the classes Socket_Manager as well as Queue_Threadsafe for their respective overviews.
//
//This file contains the specific functional UI aspects for this program.
//Other generic windows setup operations are separated out into Windows_Infrastructure.cpp
//
//Overview:
//A WM_CREATE message is sent upon creation. That case statement creates all of the boxes and buttons.
//
//WM_COMMAND -- IDC_CONNECT is called when the connect button is pressed.
//This command grabs user input for the connect command from the socket manager and calls the function.
//
//WM_COMMAND -- IDC_SEND_MESSAGE is called when the send message button is pressed or a return w/o shift.
//This command grabs user input, queues it for output, and clears the input box.
//
//WM_COMMAND -- IDC_INCOMING_MESSAGE is called by the socket-reading thread when a new message arrives.
//This command is the cue to read the incoming message queue.
//New messages are placed at the end of the output box.
//
//Other case statements are general cases for most any windows program.
//Also note the extra window procedure at the end. This is to create the enter = send functionality.
//This redirect is set up in the WM_CREATE case.
////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "stdafx.h"

//Global variables
SOCKETMANAGER sockManager;		//Socket manager through which network interface is done. This cannot be locally defined lest it be created and destroyed with each window message processed.
QUEUE_THREADSAFE inputQ;		//Make a global input queue for messages from the server to the client.
WNDPROC EditHandler;			//This is the default proceedure for ALL Windows edit boxes. Conceivably more than one edit box may need to refer back to this one proceedure.
vector<HWND> interfaceWindows, connectionWindows;

LRESULT CALLBACK WndProc(HWND hMain, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		//When the window is first opened...
	case WM_CREATE: {
		//Create all needed windows and store their handles for later use.
		interfaceWindows.push_back(CreateWindow(TEXT("edit"), TEXT(""), WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, 0, 0, 500, 500, hMain, (HMENU)ID_OUTPUT_WINDOW, hInst, NULL));
		interfaceWindows.push_back(CreateWindow(TEXT("edit"), TEXT(""), WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 0, 500, 500, 75, hMain, (HMENU)ID_INPUT_WINDOW, hInst, NULL));
		interfaceWindows.push_back(CreateWindow(TEXT("button"), TEXT("SEND"), WS_CHILD | WS_BORDER, 500, 500, 150, 75, hMain, (HMENU)IDC_SEND_MESSAGE, hInst, NULL));

		connectionWindows.push_back(CreateWindow(TEXT("button"), TEXT("CONNECT"), WS_CHILD | WS_VISIBLE | WS_BORDER, 200, 600, 100, 25, hMain, (HMENU)IDC_CONNECT, hInst, NULL));
		connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("127"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 0, 600, 50, 25, hMain, (HMENU)ID_IPv4_pt1, hInst, NULL));
		connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("0"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 50, 600, 50, 25, hMain, (HMENU)ID_IPv4_pt2, hInst, NULL));
		connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("0"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 100, 600, 50, 25, hMain, (HMENU)ID_IPv4_pt3, hInst, NULL));
		connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("1"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 150, 600, 50, 25, hMain, (HMENU)ID_IPv4_pt4, hInst, NULL));
		connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("7777"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 0, 625, 50, 25, hMain, (HMENU)ID_PORT_NUMBER, hInst, NULL));
		connectionWindows.push_back(CreateWindow(TEXT("edit"), TEXT("USER NAME"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 100, 625, 100, 25, hMain, (HMENU)ID_USERNAME, hInst, NULL));

		//Intercepts messages to input box for additional processing. Stores default procedure in EditHandler to return messages when finished.
		//This is necessary to allow sending messages with the return key.
		EditHandler = (WNDPROC)SetWindowLong(GetDlgItem(hMain, ID_INPUT_WINDOW), GWL_WNDPROC, (LONG)Input_Box_Subclass); 

	} break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDC_CONNECT: {
			//Read IP address and port number from input boxes
			string ip1 = wstring_to_string(Message_Box_to_Wstring(GetDlgItem(hMain, ID_IPv4_pt1)));
			string ip2 = wstring_to_string(Message_Box_to_Wstring(GetDlgItem(hMain, ID_IPv4_pt2)));
			string ip3 = wstring_to_string(Message_Box_to_Wstring(GetDlgItem(hMain, ID_IPv4_pt3)));
			string ip4 = wstring_to_string(Message_Box_to_Wstring(GetDlgItem(hMain, ID_IPv4_pt4)));
			string port = wstring_to_string(Message_Box_to_Wstring(GetDlgItem(hMain, ID_PORT_NUMBER)));
			string name = wstring_to_string(Message_Box_to_Wstring(GetDlgItem(hMain, ID_USERNAME)));

			//Try to connect. Upon success, hide connection input windows.
			if (sockManager.Connect_to_Server(ip1, ip2, ip3, ip4, port, name)) {
				for (auto i: interfaceWindows) { SetWindowPos(i, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE); }		//Show all interface windows
				for (auto i: connectionWindows) { SetWindowPos(i, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE); }	//Hide all connection windows
			}
		} break;
		case IDC_SEND_MESSAGE: { HWND h = GetDlgItem(hMain, ID_INPUT_WINDOW); sockManager.Push(wstring_to_string(Message_Box_to_Wstring(h))); SendMessage(hMain, WM_COMMAND, MAKEWPARAM(IDC_RESET_BOX, NULL), NULL); /*SetWindowText(h, L"");*/ } break;
		case IDC_RESET_BOX: { SetWindowText(GetDlgItem(hMain, ID_INPUT_WINDOW), L""); } break;
		case IDC_INCOMING_MESSAGE: { 
			wstring out = string_to_wstring(*inputQ.load_and_pop()); 
			out.append(L"\n");		//Adds new-line separation after each message.
			HWND h = GetDlgItem(hMain, ID_OUTPUT_WINDOW);

			//Places new messages at the end of the text in the output window.
			int sel = GetWindowTextLength(h);
			SendMessage(h, EM_SETSEL, (WPARAM)sel, (LPARAM)sel);
			SendMessage(h, EM_REPLACESEL, 0, (LPARAM)((LPWSTR)out.c_str()));
		} break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hMain, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hMain);
			break;
		default:
			return DefWindowProc(hMain, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hMain, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hMain, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hMain, message, wParam, lParam);
	}
	return 0;
}


//Literally all this does right now is catch a "Return" key press in the message entry window.
//Any other message is relayed on to the default procedure for edit boxes.
LRESULT CALLBACK Input_Box_Subclass(HWND hInputBox, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
			//
			case VK_RETURN:
			{
				//If and only if the shift-key is not pressed, cue "Send Message" as if "Send" button was pressed.
				if (GetKeyState(VK_SHIFT) >= 0) { SendMessage(GetParent(hInputBox), WM_COMMAND, IDC_SEND_MESSAGE, 0); SetWindowText(hInputBox, L""); }
			}
			break;
		}
	}
	break;
	default: return CallWindowProc(EditHandler, hInputBox, message, wParam, lParam);
	}
	return CallWindowProc(EditHandler, hInputBox, message, wParam, lParam);
}