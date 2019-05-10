
#include "ecs.h"
#include "../utils/memory_pool.h"
#include "../chunk/chunk.h"

namespace tpf_ecs
{

	//------------------------------------
	Entity::Entity()
	{
		id_ = 0;
		tag_ = 0;
		type_ = 0;
		chunk_meta_ = (ChunkMeta*)0;
	}

	Entity::~Entity()
	{
		
	}

	Component* Entity::GetComponent(int component_id)
	{
		if (!chunk_meta_)return (Component*)0;
		char* p = (char*)this;
		p -= BLOCK_META_FLAG_SIZE;	//为blockarray 做特殊处理
		return chunk_meta_->Get<Component>(p, component_id);
	}

//---------------------------------------------
	EntityManager::EntityManager()
	{
	
	}

	EntityManager::~EntityManager()
	{
	
	}

	ChunkMeta*	EntityManager::CreateEntityTypeMeta(int entity_type,int component_id_max)
	{
		std::map<int, ChunkArray*>::iterator it = entity_type_infos_.find(entity_type);
		if (it != entity_type_infos_.end()) return (ChunkMeta*)0;
		ChunkMeta* meta = ChunkMetaRegistration::CreateAndRegist(entity_type);
		meta->ResetLayout(component_id_max, BLOCK_META_FLAG_SIZE);   //为blockarray 做特殊处理
		meta->SetStructLayout(STRUCT_ID_ENTITY, sizeof(Entity));
		ChunkArray* chunk_array = new ChunkArray(meta);
		entity_type_infos_[entity_type] = chunk_array;
		return meta;
	}

	ChunkArray* EntityManager::GetChunkArrayByType(int entity_type)
	{
		std::map<int, ChunkArray*>::iterator it = entity_type_infos_.find(entity_type);
		if (it == entity_type_infos_.end()) return (ChunkArray*)0;
		return it->second;
	}

	bool EntityManager::BuildEntityArrayType(int entity_type)
	{
		std::map<int, ChunkArray*>::iterator it = entity_type_infos_.find(entity_type);
		if (it != entity_type_infos_.end())
		{
			//特殊处理,blockarray_flag 已经包含在meta了
			ChunkArray* chunk_array = (it->second);
			chunk_array->Init(chunk_array->GetMeta()->GetChuckTotalSize() - BLOCK_META_FLAG_SIZE, 128);
			return true;
		}
		return false;
	}

	Entity* EntityManager::CreateEntity(int entity_type)
	{
		Entity* ret = (Entity*)0;
		std::map<int, ChunkArray*>::iterator it =entity_type_infos_.find(entity_type);
		if (it == entity_type_infos_.end()) return ret;
		ChunkArray* chunk_array = it->second;
		
		void* p = (Entity*)(chunk_array->Create());
		ret = new (p)Entity;
		ret->chunk_meta_ = chunk_array->GetMeta();

		long long id = entity_type << 32 | (chunk_array->GetChunkUseNum() - 1);
		ret->id_ = id;
		ret->type_ = entity_type;
		return ret;
	}
	Entity* EntityManager::GetEntity(long long id)
	{
		int entity_type = id >> 32;
		int idx = (int)id;
		Entity* ret = (Entity*)0;
		std::map<int, ChunkArray*>::iterator it = entity_type_infos_.find(entity_type);
		if (it == entity_type_infos_.end())
		{
			return ret;
		}
		ChunkArray* chunk_array = it->second;
		ret = (Entity*)chunk_array->GetByIdx(idx);
		return ret;
	}
	void EntityManager::ReleaseEntity(long long id)
	{
		int entity_type = id >> 32;
		int idx = (int)id;
		Entity* ret = (Entity*)0;
		std::map<int, ChunkArray*>::iterator it = entity_type_infos_.find(entity_type);
		if (it == entity_type_infos_.end())
		{
			return;
		}
		ChunkArray* chunk_array = it->second;
		chunk_array->ReleaseByIdx(idx);
	}

	void EntityManager::RearrarChunk()
	{
		std::map<int, ChunkArray*>::iterator it = entity_type_infos_.begin();
		for (; it != entity_type_infos_.end(); ++it)
		{
			(it->second)->Rearray();
		}
	}
	
//------------------------------------
	System::System()
	{
		world_ = (EcsWorld*)0;
		tag_ = 0;
	}
	System::~System()
	{

	}
	void System::AddRefComponentId(int component_id)
	{
		ref_component_ids_.push_back(component_id);
	}

	void System::_Update(ChunkArray* chunk_array)
	{
		chunk_meta_ = chunk_array->GetMeta();
		for (int i = 0; i < ref_component_ids_.size(); ++i)
		{
			if (!chunk_meta_->IsContainStruct(ref_component_ids_.at(i)))
			{
				return;
			}
		}
		chunk_ptr_ = (char*)chunk_array->GetChunkPtr();
		chunk_block_size_ = chunk_array->GetChunkSize();
		chunk_num_ = chunk_array->GetChunkUseNum();
		if (chunk_num_ > 0)
		{
			this->OnUpdate();
		}
	}
//------------------------------------
	EcsWorld::~EcsWorld()
	{
		int num = systems_.size();
		for (int i = 0; i < num; ++i)  //或者交给业务层自己去清理
		{
			delete (systems_.at(i));
		}
		systems_.clear();
	}
	void EcsWorld::AddSystem(System* system)
	{
		system->world_ = this;
		systems_.push_back(system);
	}
	void EcsWorld::Update()
	{
		BeforeUpdate();
		GetEntityManager()->RearrarChunk();
		int num = systems_.size();
		for (int i=0;i < num;++i)
		{
			System* system = systems_.at(i);
			if( (system->ref_component_ids_).size() == 0) continue;

			for (std::map<int, ChunkArray*>::iterator iter = entity_mgr_.entity_type_infos_.begin(); \
				iter != entity_mgr_.entity_type_infos_.end(); ++iter)
			{
				ChunkArray* chunk_array = iter->second;
				(systems_.at(i))->_Update(chunk_array);
			}
		}
		AftreUpdate();
	}
} //tpf_ecs