// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <atomic>
#include <malloc.h>
#include <memory.h>
#include <mutex>
#include <queue>
#include <stdlib.h>
#include <tchar.h>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")	//Links in dll library needed for WS2tcpip.h

#include "resource.h"

using namespace std;

extern HINSTANCE hInst;
extern HWND hwnd;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Input_Box_Subclass(HWND, UINT, WPARAM, LPARAM);	//Manual pre-processing of messages to the input box to add additional functionallity.
extern WNDPROC EditHandler;		//This holds the default window proceedure for edit boxes. Any message I don't manually process will be passed along here.
extern vector<HWND> interfaceWindows, connectionWindows;

//Utilities
#include "Queue_Threadsafe.h"
extern QUEUE_THREADSAFE inputQ;		//Make a global input queue for messages from the server to the client.
#include "Utilities.h"

//Classes
#include "Socket_Manager.h"