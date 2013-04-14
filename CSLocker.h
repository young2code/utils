#pragma once

#include <windows.h>

class CSLocker
{
public:
	CSLocker(CRITICAL_SECTION* cs) : mCS(cs)
	{
		EnterCriticalSection(mCS);
	}

	~CSLocker()
	{
		LeaveCriticalSection(mCS);
	}

private:
	CSLocker(const CSLocker& rhs);
	CSLocker& operator=(CSLocker& rhs);

private:
	CRITICAL_SECTION* mCS;
};