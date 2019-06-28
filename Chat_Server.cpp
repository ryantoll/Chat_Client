/*////////////////////////////////////////////////////////////////////////////////////////////////////
//Chat_Server
//Ryan Toll 2018
//
//Contact me at:
//[last name] (dot) [first name] (at) G m A i L (dot) COM
//
//This program is the server side of an instant messenger application.
//It is designed to be paired with the corresponding client application on a private network.
//The pair is a portfolio piece meant to demonstrate competence with C++ and the Windows API.
//
//Other topics:
//Socket communication with BSD/Winsocket
//Multithreading
//UI/UX
//Class design & Object Oriented Programming
//Templates
////////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "stdafx.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	//When the window is first opened...
	case WM_CREATE: {
		CreateWindow(TEXT("edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 500, 500, hWnd, (HMENU)ID_OUTPUT_WINDOW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		CreateWindow(TEXT("edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL, 0, 500, 500, 75, hWnd, (HMENU)ID_INPUT_WINDOW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		CreateWindow(TEXT("button"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, 500, 500, 150, 75, hWnd, (HMENU)ID_INPUT_WINDOW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
	} break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
