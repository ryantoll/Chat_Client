#include "stdafx.h"

//Additional functionality for connection input boxes to make UI more intuitive.
//These represent significant quality of life improvements that even the developer will appreciate after a few tests.
LRESULT CALLBACK Connection_Box_Subclass(HWND hInputBox, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_SETFOCUS: {SendMessage(hInputBox, EM_SETSEL, 0, -1); } break;	//Default to select all when box gains focus.
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_TAB:	//Tabbing functionality: forward/reverse.
		{
			auto i = find(connectionWindows.begin(), connectionWindows.end(), hInputBox);
			//Switch directions if control button is pressed with shift.
			if (GetKeyState(VK_SHIFT) >= 0) { ++i == connectionWindows.end() ? SetFocus(*(connectionWindows.begin() + 1)) : SetFocus(*i); }
			else { i == connectionWindows.begin() + 1 ? SetFocus(connectionWindows.back()) : SetFocus(*(--i)); }
		} break;
		case VK_RETURN: { SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_CONNECT, NULL), NULL); } break;	//Press return to connect.
		}
	}
	break;
	default: return CallWindowProc(EditHandler, hInputBox, message, wParam, lParam);
	}
	return CallWindowProc(EditHandler, hInputBox, message, wParam, lParam);
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
		case VK_RETURN:
		{
			//If and only if the shift-key is not pressed, cue "Send Message" as if "Send" button was pressed.
			if (GetKeyState(VK_SHIFT) >= 0) { SendMessage(hwnd, WM_COMMAND, IDC_SEND_MESSAGE, 0); SetWindowText(hInputBox, L""); }
		}
		break;
		}
	}
	break;
	default: return CallWindowProc(EditHandler, hInputBox, message, wParam, lParam);
	}
	return CallWindowProc(EditHandler, hInputBox, message, wParam, lParam);
}