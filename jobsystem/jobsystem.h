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

	//一批并行任务
	struct Jobs
	{
		char* block;		//提高效率，数组分配连续内存
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

	//应用继承实现处理并行任务
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

	//可扩展调度实现
	class Scheduler
	{
	public:
		virtual ~Scheduler() {}
		virtual void DoJobs(Jobs* jobs, int batch_num, WorkerPool* pool) = 0;
		virtual void OnJobsDone(Jobs* jobs) = 0;
	};

	//一般分配器
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

