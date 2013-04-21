#include "Log.h"

#include "CSLocker.h"

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

	volatile bool gInitialized = false;
	volatile bool gEnable = true;

	HMODULE gLibModule;

#ifdef LOG_THREAD_SAFE
	CRITICAL_SECTION gLogCS;
#endif

	void Error(const char * fileName, const char * funcName, int line, const char * msg, ...)
	{
		if (!gInitialized || !gEnable)
		{
			return;
		}

#ifdef LOG_THREAD_SAFE
		CSLocker lock(&gLogCS);
#endif
		char buffer[BUFFER_SIZE] = {0,};
		va_list args;
		va_start(args, msg);
		vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE-1, msg, args);
		va_end(args);

		stringstream output;
		output << "File: " << fileName << "\nFunction: " << funcName << "\nLine: " << line << "\nError: " << buffer << endl;

		OutputDebugStringA(output.str().c_str());
 		cout << output.str().c_str() << endl;
	}

	void Error(const char * fileName, const char * funcName, int line, int code, const char * msg, ...)
	{
		if (!gInitialized || !gEnable)
		{
			return;
		}

#ifdef LOG_THREAD_SAFE
		CSLocker lock(&gLogCS);
#endif

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

		stringstream output;
		output << "File: " << fileName << "\nFunction: " << funcName << "\nLine: " << line << "\nError: " << buffer << "\nMsg: " << lpMessageBuffer << "Code: " << code << " 0x" << hex << code << endl;
		output << dec;

		OutputDebugStringA(output.str().c_str());
		cout << output.str().c_str() << endl;

		// Free the buffer allocated by the system.
		LocalFree( lpMessageBuffer ); 

	}

	void Log(const char * msg, ...)
	{
		if (!gInitialized || !gEnable)
		{
			return;
		}

#ifdef LOG_THREAD_SAFE
		CSLocker lock(&gLogCS);
#endif

		char buffer[BUFFER_SIZE] = {0,};
		va_list args;
		va_start(args, msg);
		vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE-1, msg, args);
		va_end(args);
			
		stringstream output;
		output << buffer << endl;

		OutputDebugStringA(output.str().c_str());
		cout << output.str().c_str() << endl;
	}

	void Init()
	{
#ifdef LOG_THREAD_SAFE
		InitializeCriticalSection(&gLogCS);
#endif

		gLibModule = LoadLibraryA("NTDLL.DLL");

		gInitialized = true;
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

		gInitialized = false;

#ifdef LOG_THREAD_SAFE
		DeleteCriticalSection(&gLogCS);
#endif
	}

	void EnableTrace(bool enable)
	{
		gEnable = enable;
	}

	bool IsEnabled()
	{
		return gEnable;
	}
}