/*////////////////////////////////////////////////////////////////////////////////////////////////////
//Chat_Server
//Ryan Toll 2018
//
//Function wrappers for short utility functions.
//"inline" specifier used to request an inline expansion of code rather than a full function call.
//This will avoid function call overhead, but will only work for small functions.
//The compiler has final discresion as to whether or not the code is actually expanded in place.
////////////////////////////////////////////////////////////////////////////////////////////////////*/

#ifndef UTILITIES
#define UTILITIES

#include "stdafx.h"

inline std::wstring string_to_wstring(const std::string input_string) {

	size_t n = input_string.size() + 1;
	std::unique_ptr<wchar_t[]> output_C_string(new wchar_t[n]);		//Dynamically allocate new array to store conversion output.
	MultiByteToWideChar(CP_UTF8, 0, input_string.c_str(), -1, output_C_string.get(), n);		//Converts input string to wide string. Function requires C-style strings (i.e. null-terminated char arrays).

	return output_C_string.get();	//The.get() is required to return a built - in pointer as opposed to a library unique ptr.
									//unique_ptr automatically de-allocates and deletes the array it allocated when it goes out of scope.
}

inline std::string wstring_to_string(const std::wstring input_wstring) {

	size_t n = input_wstring.size() + 1;
	std::unique_ptr<char[]> output_C_string(new char[n]);		//Dynamically allocate new array to store conversion output.
	WideCharToMultiByte(CP_UTF8, 0, input_wstring.c_str(), -1, output_C_string.get(), n, NULL, NULL);	//Converts input wide string to string. Function requires C-style strings (i.e. null-terminated char arrays).

	return output_C_string.get();	//The.get() is required to return a built - in pointer as opposed to a library unique ptr.
									//unique_ptr automatically de-allocates and deletes the array it allocated when it goes out of scope.
}

inline wstring Edit_Box_to_Wstring(const HWND h) {

	size_t n = GetWindowTextLength(h) + 1;
	unique_ptr<wchar_t[]>output_C_string(new wchar_t[n]);
	GetWindowText(h, output_C_string.get(), n);

	return output_C_string.get();
}

#endif // !UTILITIES

