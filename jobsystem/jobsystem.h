#pragma once


template<typename T>
class SafeQueue;
class Thread;
class Semaphore;
class Mutex;

namespace tpf_jobsystem
{
	class Scheduler;
	class JobSystem;

	struct JobHandle
	{
		Scheduler*	scheduler;
		JobSystem*	system;
	};

	//һ����������
	struct Jobs
	{
		char* block;		//���Ч�ʣ�������������ڴ�
		int   block_num;
		int   block_size;
		int   begin_idx;
		int   end_idx;
		JobHandle handle;

		Jobs()
			: block((char*)0)
			, block_num(0)
			, block_size(0)
		{

		}
	};

	//Ӧ�ü̳�ʵ�ִ���������
	class JobSystem
	{
	public:
		//overried by app
		virtual void DoJobs(Jobs* jobs, int begin_idx, int end_idx){}
	};

	class Worker
	{
	protected:
		int				 worker_id_;
		int				 is_running_;
		unsigned int	 interval_;
		Mutex*			 mutex_;
		Jobs			 jobs_;
		Thread*			 thread_;

		Semaphore*		 sem_;

		virtual void OnStart() {}
		virtual void OnStop() {}

		void Done();
	public:
		Worker();
		virtual ~Worker();

		void Start(int worker_id, unsigned int interval);
		void Stop();
		void PushJobs(Jobs* jobs);

		virtual void _DoTask();

		void Wakeup();
	};

	class WorkerPool
	{
	private:
		Worker* works_;
		int		works_num_;

		void Release();
	public:
		WorkerPool();
		~WorkerPool();
		void Start(int work_num, unsigned int interval);
		void Stop();
		int  GetWorksNum() { return works_num_; }
		Worker* GetWorkAt(int idx) 
		{
			if (idx >= works_num_) return (Worker*)0;
			return &(works_[idx]);
		}
	};

	//����չ����ʵ��
	class Scheduler
	{
	public:
		virtual ~Scheduler() {}
		virtual void DoJobs(Jobs* jobs, int batch_num, WorkerPool* pool) = 0;
		virtual void OnJobsDone(Jobs* jobs) = 0;
	};

	//һ�������
	class DefaultScheduler : public Scheduler
	{
	public:
		DefaultScheduler();
		~DefaultScheduler();

		virtual void DoJobs(Jobs* jobs, int batch_num, WorkerPool* pool);
		virtual void OnJobsDone(Jobs* jobs);

	private:
		int		   count_;
		Mutex*	   mutex_;
		
		Semaphore* sem_;

		void  AddRef();
		void  SubRef();
	};

}  //tpf_jobsystem

