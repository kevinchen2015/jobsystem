
#include "jobsystem.h"

#include "../utils/mutex.h"
#include "../utils/thread.h"
#include "../utils/safequeue.h"
#include "../utils/semaphore.h"

namespace tpf_jobsystem
{

	static void* ThreadWorkerFunction(void* param)
	{
		Worker* worker = static_cast<Worker*>(param);
		worker->_DoTask();
		return (void*)0;
	}

	//--------------------------------------------
	Worker::Worker()
	{
		mutex_ = new Mutex();
		thread_ = new Thread();
		sem_ = new Semaphore();
	}

	Worker::~Worker()
	{
		delete mutex_;
		if (thread_)
		{
			delete thread_;
			thread_ = (Thread*)0;
		}
		delete sem_;
	}

	void Worker::Start(int worker_id, unsigned int interval)
	{
		is_running_ = 0;
		worker_id_ = worker_id;
		interval_ = interval;
		int sucess = thread_->Create(&ThreadWorkerFunction, this);
		if (sucess == 1)
		{
			is_running_ = 1;
		}
		OnStart();
	}

	void Worker::Stop()
	{
		Wakeup();
		is_running_ = 0;
		thread_->Destroy();
		OnStop();
	}

	void Worker::PushJobs(Jobs* jobs)
	{
		mutex_->lock()
		jobs_ = *jobs;
		mutex_->unlock();
	}

	void Worker::_DoTask()
	{
		while (is_running_ == 1)
		{
			mutex_->lock()
			if(jobs_.block_num>0)
			{
				//do job
				((jobs_.handle).system)->DoJobs(&jobs_, jobs_.begin_idx, jobs_.end_idx);
				//callback
				((jobs_.handle).scheduler)->OnJobsDone(&jobs_);
				jobs_.block_num = 0;
			}
			mutex_->unlock();
			Done();
		}
	}


	void Worker::Done()
	{
		sem_->Wait();
	}

	void Worker::Wakeup()
	{
		sem_->Post();
	}
	//-------------------------------------------------

	WorkerPool::WorkerPool()
	{
		works_num_ = 0;
		works_ = (Worker*)0;
	}

	WorkerPool::~WorkerPool()
	{
		Release();
	}

	void WorkerPool::Release()
	{
		if (works_)
		{
			delete[] works_;
			works_ = (Worker*)0;
		}
	}

	void WorkerPool::Start(int work_num, unsigned int interval)
	{
		works_num_ = work_num;
		works_ = new Worker[works_num_];
		for (int i = 0; i < works_num_; ++i)
		{
			works_[i].Start(i, interval);
		}
	}

	void WorkerPool::Stop()
	{
		if (!works_) return;
		for (int i = 0; i < works_num_; ++i)
		{
			works_[i].Stop();
		}
		Release();
	}

	//------------------------------------------------

	DefaultScheduler::DefaultScheduler()
	{
		count_ = 0;
		mutex_ = new Mutex();
		sem_ = new Semaphore();
	}

	DefaultScheduler::~DefaultScheduler()
	{
		delete mutex_;
		delete sem_;
	}

	void  DefaultScheduler::AddRef()
	{
		mutex_->lock();
		++count_;
		mutex_->unlock();
	}

	void  DefaultScheduler::SubRef()
	{
		mutex_->lock();
		--count_;
		mutex_->unlock();
	}

	void DefaultScheduler::DoJobs(Jobs* jobs, int batch_num, WorkerPool* pool)
	{
		//写死规则
		int n = 0;
		int worker_num = pool->GetWorksNum();

		n = worker_num;
		batch_num = jobs->block_num / n;

		if (jobs->block_num <= n)
		{
			for (int i = 0; i < jobs->block_num; ++i)
			{
				AddRef();
			}
			for (int i = 0; i < jobs->block_num; ++i)
			{
				Jobs subJobs = *jobs;
				subJobs.begin_idx = i;
				subJobs.block_num = 1;
				subJobs.end_idx = i;

				pool->GetWorkAt(i)->PushJobs(&subJobs);
				pool->GetWorkAt(i)->Wakeup();
			}
		}
		else
		{
			batch_num += 1;
			int despatched = 0;
			for (int i = 0; i < n; ++i)
			{
				AddRef();
				int length = jobs->block_num - despatched < batch_num ? jobs->block_num - despatched : batch_num;
				despatched += length;
				if (despatched >= jobs->block_num)
				{
					break;
				}
			}

			despatched = 0;
			for (int i = 0; i < n; ++i)
			{
				int length = jobs->block_num - despatched < batch_num ? jobs->block_num - despatched : batch_num;

				Jobs subJobs = *jobs;
				subJobs.begin_idx = despatched;
				subJobs.block_num = length;
				subJobs.end_idx = despatched + length - 1;

				(pool->GetWorkAt(i))->PushJobs(&subJobs);
				pool->GetWorkAt(i)->Wakeup();

				despatched += length;
				if (despatched >= jobs->block_num)
				{
					break;
				}
			}
		}
		if(count_>0)
		{
			sem_->Wait();
		}
		//jobs 已经被多线程执行过了
	}
	
	void DefaultScheduler::OnJobsDone(Jobs* jobs)
	{
		SubRef();
		if (count_ == 0)
		{
			sem_->Post();
		}
	}

} //tpf_jobsystem