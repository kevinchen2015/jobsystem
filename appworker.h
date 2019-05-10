#pragma once



#include "utils/buffer.h"
#include "jobsystem/jobsystem.h"
#include "ecs/ecs.h"
#include "utils/ztime.h"
#include "chunk/chunk.h"

#include <iostream>


//偏业务层的一层组合，将底层的jobsystem和ecs 做一层基本业务封装

//--------------------------------------------------------

namespace tpf_app
{
	#define JOB_BATCH_NUM 64

	using namespace tpf_jobsystem;
	using namespace tpf_ecs;

	class AppSystem : public System
	{
	protected:
		friend class AppWorker;
		
		JobSystem*	jobs_system_;
		
	public:
		WorkerPool* pool_;

		AppSystem()
		{
			jobs_system_ = (JobSystem*)0;
		}

		virtual ~AppSystem()
		{
			if(jobs_system_)delete jobs_system_;
		}

		void SetJobSystem(JobSystem* jobsystem)
		{
			jobs_system_ = jobsystem;
		}

		virtual void OnUpdate()
		{
			Jobs jobs;
			DefaultScheduler scheduler;
			jobs.block = chunk_ptr_;
			jobs.block_num = chunk_num_;
			jobs.block_size = chunk_block_size_;
			jobs.begin_idx = 0;
			jobs.end_idx = chunk_num_ - 1;

			if (jobs.block_num > 0)
			{
				jobs.handle.scheduler = &scheduler;
				jobs.handle.system = jobs_system_;
				scheduler.DoJobs(&jobs, JOB_BATCH_NUM, pool_);
			}	
		}
	};

	//----------------------------------------------------------
}