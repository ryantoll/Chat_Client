#include "stdafx.h"

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