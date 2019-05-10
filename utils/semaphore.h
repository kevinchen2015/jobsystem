#pragma once

#ifdef WIN32
#include <Windows.h>


class Semaphore
{
public:

	Semaphore()
	{
		sem_ = ::CreateSemaphore(NULL, 0, 1, NULL);
	}
	~Semaphore()
	{
		CloseHandle(sem_);
	}

	int Post()
	{
		::ReleaseSemaphore(sem_, 1, NULL);
		return 0;
	}
	int Wait()
	{
		::WaitForSingleObject(sem_, INFINITE);
		return 0;
	}
private:
	HANDLE sem_;
};


#else   

#include <semaphore.h>

class Semaphore
{
public:
	
	Semaphore()
	{
		int ret = sem_init(&sem_, 0, 0);
	}
	~Semaphore()
	{
		sem_destroy(&sem_);
	}

	int Post()
	{
		sem_post(&sem_);
		return 0;
	}
	int Wait()
	{
		sem_wait(&sem_);
		return 0;
	}

private:
	sem_t sem_;
};

#endif

