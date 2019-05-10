
#include "chunk.h"
#include "../utils/memory_pool.h"

namespace tpf_chunk
{
		//----------------------------------------------------	
		std::map<int, ChunkMeta*> ChunkMetaRegistration::type2chunkmeta_;
		ChunkMeta* ChunkMetaRegistration::CreateAndRegist(int chunk_type)
		{
			ChunkMeta* meta = GetChunkMetaByType(chunk_type);
			if (meta) return meta;
			meta = new ChunkMeta();
			type2chunkmeta_[chunk_type] = meta;
			return meta;
		}
		void ChunkMetaRegistration::Release(int chunk_type)
		{
			ChunkMeta* meta = GetChunkMetaByType(chunk_type);
			type2chunkmeta_.erase(chunk_type);
			//if (meta) delete meta;  //≤ª Õ∑≈
		}
		ChunkMeta* ChunkMetaRegistration::GetChunkMetaByType(int chunk_type)
		{
			std::map<int, ChunkMeta*>::iterator iter = type2chunkmeta_.find(chunk_type);
			if (iter != type2chunkmeta_.end())return iter->second;
			return (ChunkMeta*)0;
		}
	
		//----------------------------------------------------
		ChunkArray::ChunkArray(ChunkMeta* meta)
		{
			meta_ = meta;
			array_ = new BlockArrayPool();
		}
		ChunkArray::~ChunkArray()
		{
			delete array_;
		}
		void ChunkArray::Init(int chunk_size, int chunk_num)
		{
			array_->Init(chunk_size, chunk_num);
		}
		void* ChunkArray::Create()
		{
			void* p = array_->AllocBlock();
			return p;
		}
		void ChunkArray::Release(void* p)
		{
			array_->FreeBlock(p);
		}
		void ChunkArray::ReleaseByIdx(int idx)
		{
			array_->FreeBlockByIndex(idx);
		}
		void* ChunkArray::GetByIdx(int idx)
		{
			return array_->GetBlockByIndex(idx);
		}
		void ChunkArray::Rearray()
		{
			array_->Rearray();
		}
		void* ChunkArray::GetChunkPtr()
		{
			return array_->GetChunkPtr();
		}
		int  ChunkArray::GetChunkSize()
		{
			return array_->GetRealBlockSize();
		}
		int  ChunkArray::GetChunkUseNum()
		{
			return array_->GetBlockUseNum();
		}
}