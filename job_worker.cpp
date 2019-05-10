// test_jobsystem.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#if 1

#include "appworker.h"
#include "utils/safequeue.h"
#include "utils/thread.h"
#include "utils/zmemory.h"
#include "utils/memory_pool.h"
#include "chunk/chunk.h"
#include "ecs/ecs.h"

#include "job_worker.h"

#include <Box2D/Box2D.h>

#include <iostream>
#include <map>


//--------------------------移动和碰撞业务实现--------------------

using namespace tpf_jobsystem;
using namespace tpf_ecs;
using namespace tpf_app;


//-----------------------------------------------------

//空间数据
struct SpaceComponent : public Component
{
	int   id;
	b2Vec2 pos;
	float angle;
	float speed;
	float hx, hy;
	//and so on...
};


//碰撞数据
struct CollisionComponent : public Component
{
	int hit_id;
};


//--------------------------------------------------------


void PushToWorker(GameCmd& cmd);

//move jobsystem
class MoveJobSystem : public JobSystem
{
public:

	virtual void DoJobs(Jobs* jobs, int begin_idx, int end_idx)
	{
		float dt_ = 0.033f;
		//std::cout << "MoveJobSystem, begin:" << begin_idx << " end:" << end_idx << std::endl;
		char* ptr = jobs->block;
		int cnt = 0;
		for (int i = begin_idx; i < jobs->block_num + begin_idx; ++i,ptr += jobs->block_size)
		{
			Entity* e = (Entity*)(ptr);
			SpaceComponent* space = static_cast<SpaceComponent*>(e->GetComponent(SPACE_COMPONENT));
			
			b2Vec2 v(0.0f, space->speed * dt_);
			b2Rot rot(space->angle);
			v = b2Mul(rot, v);
			space->pos = space->pos + v;
			//std::cout << "pos x:" << space->pos.x << " y:" << space->pos.y << std::endl;
		}
	}
};

//collision jobsystem
class CollisionJobSystem : public JobSystem
{
	
public:
	EcsWorld* world_;
	virtual void DoJobs(Jobs* jobs, int begin_idx, int end_idx)
	{
		//std::cout << "CollisionJobSystem, begin:"<<begin_idx <<" end:"<<end_idx<< std::endl;
		char* ptr = jobs->block;
		int cnt = 0;
		ChunkArray* chunk_array = world_->GetEntityManager()->GetChunkArrayByType(FISH);
		for (int i = begin_idx; i < jobs->block_num + begin_idx; ++i, ptr += jobs->block_size)
		{
			Entity* e = (Entity*)(ptr);
			SpaceComponent* bullet_space = static_cast<SpaceComponent*>(e->GetComponent(SPACE_COMPONENT));
			CollisionComponent* collision = static_cast<CollisionComponent*>(e->GetComponent(COLLISION_COMPONENT));

			if (collision->hit_id != 0) continue; //已经碰撞上了

			b2PolygonShape bullet_shape;
			bullet_shape.SetAsBox(bullet_space->hx, bullet_space->hy, bullet_space->pos, bullet_space->angle);
			b2Transform t;
			int fish_block_size_ = chunk_array->GetChunkSize();
			char* fish_ptr = (char*)chunk_array->GetChunkPtr();
			for (int m = 0; m < chunk_array->GetChunkUseNum(); ++m)
			{
				Entity* fish = (Entity*)(fish_ptr + m*fish_block_size_);
				SpaceComponent* fish_space = static_cast<SpaceComponent*>(fish->GetComponent(SPACE_COMPONENT));

				//检测
				b2PolygonShape fish_shape;
				fish_shape.SetAsBox(fish_space->hx, fish_space->hy, fish_space->pos, fish_space->angle);
				
				bool hit = false;
				for (int n = 0; n < bullet_shape.m_count; ++n)
				{
					if (fish_shape.TestPoint(t, bullet_shape.m_vertices[n]))
					{
						hit = true;
						break;
					}
				}
				if (hit)
				{
					collision->hit_id = fish_space->id;
					std::cout << " collision hit,id:" << collision->hit_id << std::endl;
					break;
				}
			}
		}
	}
};

//--------------------------------------------------------

class MoveSystem : public AppSystem
{
	TimeProfiler  parallel_profiler;
	TimeProfiler  serial_profiler;
public:

	virtual void OnUpdate()
	{
		//TimeProfilerStart(&parallel_profiler);
		AppSystem::OnUpdate();
		//TimeProfilerEnd(&parallel_profiler);

		/*
		TimeProfilerStart(&serial_profiler);
		Jobs jobs;
		DefaultScheduler scheduler;
		jobs.block = chunk_ptr_;
		jobs.block_num = chunk_num_;
		jobs.block_size = chunk_block_size_;
		jobs.begin_idx = 0;
		jobs.end_idx = chunk_num_ - 1;
		this->jobs_system_->DoJobs(&jobs, jobs.begin_idx, jobs.end_idx);
		TimeProfilerEnd(&serial_profiler);

		std::cout << " parallel_profiler:" << parallel_profiler.time_cost << " serial_profiler:" << serial_profiler.time_cost << std::endl;
		*/
	}
};

class CollisionSystem : public AppSystem
{
	TimeProfiler  parallel_profiler;
	TimeProfiler  serial_profiler;
public:
	virtual void OnUpdate()
	{
		//TimeProfilerStart(&parallel_profiler);
		AppSystem::OnUpdate();
		//TimeProfilerEnd(&parallel_profiler);

		/*
		TimeProfilerStart(&serial_profiler);
		Jobs jobs;
		DefaultScheduler scheduler;
		jobs.block = chunk_ptr_;
		jobs.block_num = chunk_num_;
		jobs.block_size = chunk_block_size_;
		jobs.begin_idx = 0;
		jobs.end_idx = chunk_num_ - 1;
		this->jobs_system_->DoJobs(&jobs, jobs.begin_idx, jobs.end_idx);
		TimeProfilerEnd(&serial_profiler);

		std::cout << " parallel_profiler:" << parallel_profiler.time_cost << " serial_profiler:" << serial_profiler.time_cost << std::endl;
		*/
	}
};


class SyncMoveSystem : public AppSystem
{
public:
	virtual void OnUpdate()
	{
		//custom todo
		char* ptr = chunk_ptr_;
		for (int i = 0; i < chunk_num_; ++i)
		{
			Entity* e = (Entity*)(ptr);
			ptr += chunk_block_size_;
			SpaceComponent* space = static_cast<SpaceComponent*>(e->GetComponent(SPACE_COMPONENT));
			
			GameCmd cmd;
			cmd.cmd_id = SYNC_MOVE;
			cmd.sync_move.id = space->id;
			cmd.sync_move.x = space->pos.x;
			cmd.sync_move.y = space->pos.y;
			JobCmdPushToGame(cmd);
		}
	}
};

class SyncCollisionSystem : public AppSystem
{
public:

	virtual void OnUpdate()
	{
		//custom todo
		char* ptr = chunk_ptr_;
		for (int i = 0; i < chunk_num_; ++i)
		{
			Entity* e = (Entity*)(ptr);
			ptr += chunk_block_size_;
			CollisionComponent* collision = static_cast<CollisionComponent*>(e->GetComponent(COLLISION_COMPONENT));
			if (collision->hit_id != 0)
			{
				SpaceComponent* space = static_cast<SpaceComponent*>(e->GetComponent(SPACE_COMPONENT));
				GameCmd cmd;
				cmd.cmd_id = SYNC_COLLID;
				cmd.sync_collider.id = space->id;
				cmd.sync_collider.hit_id = collision->hit_id;
				JobCmdPushToGame(cmd);
			}
		}
	}
};

//--------------------------------------------

class MainWorker : public Worker , public EcsWorld
{
	SafeQueue<GameCmd>*		 game_cmd_in_;
	bool					 need_update_;
	WorkerPool				 workerpool_;
	std::map<int, long long> id2entityid_;
public:
	int worker_num_;
	SafeQueue<GameCmd>* game_cmd_out_;

	MainWorker() :Worker()
	{
		worker_num_ = 4;
		game_cmd_in_ = new SafeQueue<GameCmd>();
		game_cmd_out_ = new SafeQueue<GameCmd>();
		need_update_ = false;
	}

	void RegistSystem(AppSystem* system)
	{
		if (!system)return;
		system->pool_ = &workerpool_;
		this->AddSystem(system);
	}

	virtual void BeforeUpdate()
	{
	}
	virtual void AftreUpdate()
	{
	}

	virtual void OnStart()
	{
		Worker::OnStart();
		workerpool_.Start(worker_num_, 1);

		EntityManager* mgr = this->GetEntityManager();

		//regist fish entity
		ChunkMeta* meta = mgr->CreateEntityTypeMeta(FISH, MAX_COMPNET_NUM);
		meta->SetStructLayout(SPACE_COMPONENT, sizeof(SpaceComponent));
		mgr->BuildEntityArrayType(FISH);

		//regist bullet entity
		meta = mgr->CreateEntityTypeMeta(BULLET, MAX_COMPNET_NUM);
		meta->SetStructLayout(SPACE_COMPONENT, sizeof(SpaceComponent));
		meta->SetStructLayout(COLLISION_COMPONENT, sizeof(CollisionComponent));
		mgr->BuildEntityArrayType(BULLET);

		//add system
		AppSystem* p = new MoveSystem();
		p->AddRefComponentId(SPACE_COMPONENT);
		p->SetJobSystem(new MoveJobSystem());
		this->RegistSystem(p);

		p = new CollisionSystem();
		p->AddRefComponentId(COLLISION_COMPONENT);
		p->AddRefComponentId(SPACE_COMPONENT);

		CollisionJobSystem* collision_job_system = new CollisionJobSystem();
		collision_job_system->world_ = this;
		p->SetJobSystem(collision_job_system);
		this->RegistSystem(p);

		p = new SyncMoveSystem();
		p->AddRefComponentId(SPACE_COMPONENT);
		this->RegistSystem(p);

		p = new SyncCollisionSystem();
		p->AddRefComponentId(COLLISION_COMPONENT);
		this->RegistSystem(p);
	}

	virtual void OnStop()
	{
		workerpool_.Stop();
		Worker::OnStop();
	}

	virtual ~MainWorker()
	{
		
	}

	virtual void _DoTask()
	{
		while (is_running_ == 1)
		{
			GameCmd cmd;
			while(game_cmd_in_->Pop(cmd))
			{
				switch (cmd.cmd_id)
				{
					case CMD_TICK:
					need_update_ = true;
					break;

					case ADD_OBJ:
					{
						if (cmd.add_obj.type == FISH)
						{
							Entity* e = this->GetEntityManager()->CreateEntity(FISH);
							SpaceComponent* space = static_cast<SpaceComponent*>(e->GetComponent(SPACE_COMPONENT));
							space->id = cmd.add_obj.id;
							space->pos.Set(cmd.add_obj.x, cmd.add_obj.y);
							space->speed = 2.0f;
							space->speed = cmd.add_obj.speed;
							space->angle = cmd.add_obj.angle;
							space->hx = cmd.add_obj.w / 2;
							space->hy = cmd.add_obj.h / 2;
							
							id2entityid_[space->id] = e->GetId();
						}
						else if (cmd.add_obj.type == BULLET)
						{
							Entity* e = this->GetEntityManager()->CreateEntity(BULLET);
							SpaceComponent* space = static_cast<SpaceComponent*>(e->GetComponent(SPACE_COMPONENT));
							space->id = cmd.add_obj.id;
							space->pos.Set(cmd.add_obj.x, cmd.add_obj.y);
							space->speed = cmd.add_obj.speed;
							space->angle = cmd.add_obj.angle;
							space->hx = cmd.add_obj.w/2;
							space->hy = cmd.add_obj.h/2;
							CollisionComponent* collision = static_cast<CollisionComponent*>(e->GetComponent(COLLISION_COMPONENT));
							collision->hit_id = 0;

							id2entityid_[space->id] = e->GetId();
						}
					}
					break;

					case REMOVE_OBJ:
					{
						std::map<int, long long>::iterator it = id2entityid_.find(cmd.remove_obj.id);
						if (it != id2entityid_.end())
						{
							long long e_id = it->second;
							id2entityid_.erase(cmd.remove_obj.id);
							this->GetEntityManager()->ReleaseEntity(e_id);
						}
					}
					break;

					case MODIFY_OBJ:
					{
						std::map<int, long long>::iterator it = id2entityid_.find(cmd.modify_obj.id);
						if (it != id2entityid_.end())
						{
							long long e_id = it->second;
							Entity* e = this->GetEntityManager()->GetEntity(e_id);
							if (e)
							{
								SpaceComponent* space = static_cast<SpaceComponent*>(e->GetComponent(SPACE_COMPONENT));
								space->speed = cmd.modify_obj.speed;
								space->angle = cmd.modify_obj.angle;
							}
						}
					}
					break;
				}
			}
			if (need_update_)
			{
				//update ecs and scheduler jobsystem
				this->Update();
				need_update_ = false;
			}
			Thread_Sleep(interval_);
		}
	}

	void PushToGame(GameCmd& cmd)
	{
		game_cmd_out_->Push(cmd);
	}

	void PushToWorker(GameCmd& cmd)
	{
		game_cmd_in_->Push(cmd);
	}

	void DoTest1()
	{
		for (int i = 0; i < 1; ++i)
		{
			Entity* e = this->GetEntityManager()->CreateEntity(BULLET);
			SpaceComponent* space = static_cast<SpaceComponent*>(e->GetComponent(SPACE_COMPONENT));
			space->id = i;
			space->pos.Set(100.0f, 100.0f);
			space->speed = 2.0f;
			space->angle = 45.0f;
			space->hx = 5.0f;
			space->hy = 10.0f;
			CollisionComponent* collision = static_cast<CollisionComponent*>(e->GetComponent(COLLISION_COMPONENT));
			collision->hit_id = 0;
		}
	}

	void DoTest3()
	{
		for (int i = 0; i < 1; ++i)
		{
			Entity* e = this->GetEntityManager()->CreateEntity(FISH);
			SpaceComponent* space = static_cast<SpaceComponent*>(e->GetComponent(SPACE_COMPONENT));
			space->id = i + 10000;
			space->pos.Set(200.0f, 200.0f);
			space->speed = 2.0f;
			space->angle = 0.1f;
			space->hx = 10.0f;
			space->hy = 30.0f;
		}
	}

	void DoTest2()
	{
		need_update_ = true;
	}
protected:



private:

};

static MainWorker main_worker;
void PushToWorker(GameCmd& cmd)
{
	main_worker.PushToWorker(cmd);
}

//------------------------------------

void JobWorkerInit(int interval, int thread_num)
{
	main_worker.worker_num_ = thread_num;
	main_worker.Start(1000, interval);
	//main_worker.DoTest3();
}

void JobWorkerUninit()
{
	main_worker.Stop();
}

void JobCmdPushToGame(GameCmd& cmd)
{
	main_worker.PushToGame(cmd);
}

void JobWorkerUpdate(OnRecvFunc cb)
{
	GameCmd cmd;
	while (main_worker.game_cmd_out_->Pop(cmd))
	{
		if (cb)
		{
			cb(cmd);
		}
	}
	{
		GameCmd c;
		c.cmd_id = CMD_TICK;
		main_worker.PushToWorker(c);
	}
}


#else


#endif
