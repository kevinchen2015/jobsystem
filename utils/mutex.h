#pragma once
#ifdef WIN32
#include <Windows.h>
#include <process.h>

class  Mutex
{
public:
    Mutex() { InitializeCriticalSection(&m_CriticalSection); }
    ~Mutex() { DeleteCriticalSection(&m_CriticalSection); }
    int lock() { EnterCriticalSection(&m_CriticalSection); return 1; }
    int unlock() { LeaveCriticalSection(&m_CriticalSection); return 1; }
private:
    CRITICAL_SECTION m_CriticalSection;
};

#else   // POSIX

#include <pthread.h>

class Mutex
{
public:
	Mutex() { pthread_mutex_init(&m_Mutex, NULL); }
    ~Mutex() { pthread_mutex_destroy(&m_Mutex); }
    int lock() { int nRetCode = pthread_mutex_lock(&m_Mutex);      return (nRetCode == 0)?1:0; }
    int unlock() { int nRetCode = pthread_mutex_unlock(&m_Mutex);    return (nRetCode == 0)?1:0; }

private:
    pthread_mutex_t m_Mutex;
};

#endif

