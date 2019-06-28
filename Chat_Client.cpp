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
vector<HWND> interfaceWindows, connectionWindows;
WNDPROC EditHandler;			//This is the default proceedure for ALL Windows edit boxes. Conceivably more than one edit box may need to refer back to this one proceedure.
QUEUE_THREADSAFE inputQ;		//Make a global input queue for messages from the server to the client.

LRESULT CALLBACK WndProc(HWND hMain, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		//When the window is first opened...
	case WM_CREATE: { hwnd = hMain;	Setup_Window_Layout(); } break;		//hwnd is also defined in Windows_Infrastructure, but this create command runs first. It needs to be defined before Setup_Window_Layout().
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId) {
			case IDC_CONNECT: {	sockManager.Connect_to_Server(); } break;
			case IDC_SEND_MESSAGE: { sockManager.Queue_Message(); } break;
			case IDC_RESET_BOX: { SetWindowText(GetDlgItem(hMain, ID_INPUT_WINDOW), L""); } break;
			case IDC_INCOMING_MESSAGE: { Receive_Message(); } break;
			case IDM_ABOUT: { DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hMain, About); } break;
			case IDM_EXIT: { DestroyWindow(hMain); } break;
			default: { return DefWindowProc(hMain, message, wParam, lParam); }
		}
	} break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hMain, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hMain, &ps);
	} break;
	case WM_DESTROY: { PostQuitMessage(0); } break;
	default: { return DefWindowProc(hMain, message, wParam, lParam); }	
	}
	return 0;
}