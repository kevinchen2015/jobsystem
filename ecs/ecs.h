#pragma once

#include <map>
#include <list>
#include <vector>


class SimplePool;
typedef void(*BlockReleaseCb)(void*);

namespace tpf_chunk
{
	class ChunkMeta;
	class ChunkArray;
}



namespace tpf_ecs
{


#define INVAILD_ENTITY_ID 0
#define INVAILD_COMPONENT_ID -1
#define STRUCT_ID_ENTITY 0				//实体结构
#define STRUCT_ID_BLOCK_ARRAY_FLAG 1
#define STRUCT_ID_RESERVE_2 2
#define COMPONENT_ID_BEGIN  STRUCT_ID_RESERVE_2 + 1


	using namespace tpf_chunk;

	class Entity;
	class EntityManager;
	class EcsWorld;

	//业务扩展，纯数据,不能有虚函数
	struct Component
	{
		Entity* entity;
	};

	//一类实体,不能含虚函数，有一类相同的组件数据集合,连续的整块内存，提高内存命中效率
	class Entity
	{
		long long id_;
		int tag_;
		int type_;

		ChunkMeta* chunk_meta_;
		friend class EntityManager;

		Entity();
		~Entity();

	public:
		Component* GetComponent(int component_id);
		long long  GetId() { return id_; }
		int  GetType() { return type_; }
		void SetTag(int tag) { tag_ = tag; }
		int  GetTag() { return tag_; }
	};

	class EntityManager
	{
		std::map<int, ChunkArray*> entity_type_infos_;
		EcsWorld* world_;
		friend class EcsWorld;
	public:
		EntityManager();
		~EntityManager();
	
		ChunkMeta*	CreateEntityTypeMeta(int entity_type,int component_id_max);
		bool	BuildEntityArrayType(int entity_type);

		Entity*	CreateEntity(int entity_type);
		Entity* GetEntity(long long id);
		void	ReleaseEntity(long long id);
		void    RearrarChunk();

		ChunkArray* GetChunkArrayByType(int entity_type);
	};

	//业务扩展，对某个组件集合做处理
	class System
	{
	protected:
		std::vector<int> ref_component_ids_;
		EcsWorld* world_;
		friend class EcsWorld;
		ChunkMeta* chunk_meta_;
		char*      chunk_ptr_;
		int		   chunk_block_size_;
		int		   chunk_num_;
		int tag_;
		void _Update(ChunkArray* chunk_array);
	public:
		System();
		virtual ~System();
		void AddRefComponentId(int component_id);
		virtual void OnUpdate() = 0;
		void SetTag(int tag) { tag_ = tag; }
		int  GetTag() { return tag_; }
	};

	class EcsWorld
	{
		EntityManager		  entity_mgr_;
		std::vector<System*>  systems_;
	public:
		~EcsWorld();
		EntityManager* GetEntityManager() {return &entity_mgr_;}
		void AddSystem(System* system);
		void Update();
		virtual void BeforeUpdate() {}
		virtual void AftreUpdate() {}
	};

}  //tpf_ecs

