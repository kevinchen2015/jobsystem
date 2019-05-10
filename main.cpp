

#include "stdafx.h"

#include "utils/thread.h"
#include <iostream>

#if 1


#include "job_worker.h"

void OnRecvJobSyncCmd(GameCmd& cmd)
{
	if (cmd.cmd_id == SYNC_MOVE)
	{
	float x = cmd.sync_move.x;
	float y = cmd.sync_move.y;
	std::cout << "pos x:" << x << " y:" << y << std::endl;
	}
	else if (cmd.cmd_id == SYNC_COLLID)
	{

	}
}

int main()
{
	int size = sizeof(GameCmd);
	JobWorkerInit(4, 4);
	for (;;)
	{
		JobWorkerUpdate(OnRecvJobSyncCmd);
		Thread_Sleep(33);
	}

    return 0;
}

#else

static void* ThreadWorkerFunction(void* param)
{
	while (true)
	{
		hostent* p = gethostbyname("www.qq.com");
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		SOCKADDR_IN sin;
		sin.sin_addr = *(IN_ADDR *)p->h_addr;
		sin.sin_family = AF_INET;
		
		Thread_Sleep(1);
	}
}

Thread thread[20];
int main()
{
	for (int i = 0; i < 20; ++i)
	{
		thread[i].Create(ThreadWorkerFunction, (void*)0);
	}
}

#endif
