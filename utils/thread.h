#pragma once

typedef void* ThreadFunction(void *pvArg);

#ifdef WIN32
#include "process.h"
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif


//---------------------------------------------------------------------------
class  Thread
{
public:
    int             Create(ThreadFunction *pfnThread, void *pvArg);
    int             Destroy();

public:
	Thread();
	~Thread();

    void            _ThreadFunction();

private:
    #ifdef WIN32
    HANDLE          m_ThreadHandle;
    #else
    pthread_t       m_ThreadHandle;  
    #endif

	ThreadFunction *m_pfnThread;
    void           *m_pvThreadArg;
};

 int  Thread_Sleep(unsigned int uMilliseconds);

#ifdef WIN32

inline void *Thread_GetSelfId()
{
    return (void *)((char *)NULL + GetCurrentThreadId());   // for no 64 bit check warning
}

#else     //linux

inline void *Thread_GetSelfId()
{
    return (void *)pthread_self();
}

#endif

