#pragma once

#define NOMINMAX
#include <windows.h>
#include <string>
#include <cassert>

#define CHECK(condition) assert(condition)

#ifdef _DEBUG

// https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
inline std::string GetLastErrorAsString()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

#define CHECK_WIN_CALL(condition) do { if (!(condition)) { printf("Windows error: %s\n", GetLastErrorAsString().c_str()); assert(condition); }} while(false)

#define DCHECK(condition)	CHECK(condition)

#else

#define CHECK_WIN_CALL(condition)

#define DCHECK(condition)

#endif