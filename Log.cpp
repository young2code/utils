#include "Log.h"

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

#pragma warning(disable:4101) // warning C4101: 'lock' : unreferenced local variable

namespace Log
{
	const int BUFFER_SIZE = 1024;

	bool gEnable = true;

	HMODULE gLibModule;

#ifdef LOG_THREAD_SAFE
	CRITICAL_SECTION gLogCS;
#endif

	class GlobalLocker
	{
#ifdef LOG_THREAD_SAFE
	public:
		Locker()
		{
			EnterCriticalSection(&gLogCS);
		}

		~Locker()
		{
			LeaveCriticalSection(&gLogCS);
		}
#endif
	};

	void Error(const char * fileName, const char * funcName, int line, const char * msg, ...)
	{
		GlobalLocker lock;

		char buffer[BUFFER_SIZE] = {0,};
		va_list args;
		va_start(args, msg);
		vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE-1, msg, args);
		va_end(args);

		wstringstream output;
		output << "File: " << fileName << "\nFunction: " << funcName << "\nLine: " << line << "\nError: " << buffer << endl;

		OutputDebugString(output.str().c_str());

	}

	void Error(const char * fileName, const char * funcName, int line, int code, const char * msg, ...)
	{
		GlobalLocker lock;

		char* lpMessageBuffer;

		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_FROM_HMODULE,
			gLibModule, 
			code,  
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR) &lpMessageBuffer,  
			0,  
			NULL );

		char buffer[BUFFER_SIZE] = {0,};
		va_list args;
		va_start(args, msg);
		vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE-1, msg, args);
		va_end(args);		

		wstringstream output;
		output << "File: " << fileName << "\nFunction: " << funcName << "\nLine: " << line << "\nError: " << buffer << "\nMsg: " << lpMessageBuffer << "Code: " << code << " 0x" << hex << code << endl;
		output << dec;

		OutputDebugString(output.str().c_str());

		// Free the buffer allocated by the system.
		LocalFree( lpMessageBuffer ); 

	}

	void Log(const char * msg, ...)
	{
		if( gEnable )
		{
			GlobalLocker lock;
	
			char buffer[BUFFER_SIZE] = {0,};
			va_list args;
			va_start(args, msg);
			vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE-1, msg, args);
			va_end(args);
			
			wstringstream output;
			output << buffer << endl;

			OutputDebugString(output.str().c_str());
		}
	}

	void Init()
	{
#ifdef LOG_THREAD_SAFE
		InitializeCriticalSection(&gLogCS);
#endif

		gLibModule = LoadLibraryA("NTDLL.DLL");
	}

	void Shutdown()
	{
#ifdef LOG_THREAD_SAFE
		EnterCriticalSection(&gLogCS);
#endif
		if(false == FreeLibrary(gLibModule))
		{
			ERROR_CODE(GetLastError(), "Log::CleanUp() - FreeLibrary() failed.");
		}

#ifdef LOG_THREAD_SAFE
		DeleteCriticalSection(&gLogCS);
#endif
	}

	void EnableTrace(bool enable)
	{
		gEnable = enable;
	}
}