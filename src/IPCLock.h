#pragma once

#define DEBUG_LOCK

class CCritSec 
{
	CCritSec(const CCritSec &refCritSec);
	CCritSec &operator=(const CCritSec &refCritSec);

	CRITICAL_SECTION m_CritSec;

#ifdef DEBUG_LOCK
public:
	DWORD   m_currentOwner;
	DWORD   m_lockCount;
	BOOL    m_fTrace; 
public:
	CCritSec();
	~CCritSec();
	void Lock();
	void Unlock();

	LOG_CLS_DEC();
#else

public:
	CCritSec() 
	{
		InitializeCriticalSection(&m_CritSec);
	};

	~CCritSec() 
	{
		DeleteCriticalSection(&m_CritSec);
	};

	void Lock()
	{
		EnterCriticalSection(&m_CritSec);
	};

	void Unlock()
	{
		LeaveCriticalSection(&m_CritSec);
	};
#endif
};

//
// To make deadlocks easier to track it is useful to insert in the
// code an assertion that says whether we own a critical section or
// not.  We make the routines that do the checking globals to avoid
// having different numbers of member functions in the debug and
// retail class implementations of CCritSec.  In addition we provide
// a routine that allows usage of specific critical sections to be
// traced.  This is NOT on by default - there are far too many.
//

#ifdef DEBUG_LOCK
BOOL WINAPI CritCheckIn(CCritSec * pcCrit);
BOOL WINAPI CritCheckIn(const CCritSec * pcCrit);
BOOL WINAPI CritCheckOut(CCritSec * pcCrit);
BOOL WINAPI CritCheckOut(const CCritSec * pcCrit);
void WINAPI DbgLockTrace(CCritSec * pcCrit, BOOL fTrace);
#else
#define CritCheckIn(x) TRUE
#define CritCheckOut(x) TRUE
#define DbgLockTrace(pc, fT)
#endif


// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class CAutoLock
{
	CAutoLock(const CAutoLock &refAutoLock);
	CAutoLock &operator=(const CAutoLock &refAutoLock);

protected:
	CCritSec * m_pLock;

public:
	CAutoLock(CCritSec * plock)
	{
		m_pLock = plock;
		m_pLock->Lock();
	};

	~CAutoLock() 
	{
		m_pLock->Unlock();
	};
};