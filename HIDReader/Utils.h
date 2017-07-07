#pragma once

#include <Windows.h>
#include <iostream>
#include <string>

namespace kat
{
	inline LPTSTR GetErrorMessage(DWORD errCode = GetLastError())
	{
		LPTSTR buf = NULL;
		auto ret = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			errCode,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(LPWSTR)&buf,
			0,
			NULL);
		return buf;
	}

	inline void PrintErrorMessage(DWORD errCode = GetLastError())
	{
		std::wstring s = GetErrorMessage();
		std::wcerr << "[ERROR " << errCode << "]: ";
		std::wcerr << s << std::endl;
	}
}