#pragma once

#include <map>

class BlockArrayPool;
namespace tpf_chunk
{
	struct StructLayout
	{
		int struct_size;
		int struct_offset;

		StructLayout()
		{
			struct_size = 0;
			struct_offset = -1;
		}
	};

	class ChunkMeta
	{
		int			  structs_num_;
		StructLayout* structs_layout_;
		int			  total_size_;
		int			  offset_;
	public:
		ChunkMeta()
		{
			structs_num_ = 0;
			structs_layout_ = (StructLayout*)0;
			total_size_ = 0;
			offset_ = 0;
		}

		~ChunkMeta()
		{
			if (structs_layout_)
			{
				delete[] structs_layout_;
			}
		}

		void ResetLayout(int struct_id_max,int offset=0)
		{
			if (structs_layout_)
			{
				delete[] structs_layout_;
			}
			total_size_ = 0;
			offset_ = offset;
			structs_num_ = struct_id_max;
			structs_layout_ = new StructLayout[structs_num_];
		}

		ChunkMeta* SetStructLayout(int struct_id, int struct_size)
		{
			if (struct_id >= structs_num_) return this;
			structs_layout_[struct_id].struct_size = struct_size;
			structs_layout_[struct_id].struct_offset = offset_;
			offset_ += struct_size;
			total_size_ = offset_;
			return this;
		}

		int GetChuckTotalSize()
		{
			return total_size_;
		}

		bool IsContainStruct(int struct_id)
		{
			if (struct_id >= structs_num_) return false;
			if (structs_layout_[struct_id].struct_offset < 0) return false;
			return true;
		}

		StructLayout* GetStructLayout(int struct_id)
		{
			StructLayout* ret = (StructLayout*)0;
			if (struct_id >= structs_num_) return (StructLayout*)0;
			ret = &structs_layout_[struct_id];
			if(ret->struct_offset < 0) return (StructLayout*)0;
			return ret;
		}

		template<typename T>
		T* Get(void* ptr,int struct_id)
		{
			StructLayout* layout = GetStructLayout(struct_id);
			if (!layout) return (T*)0;
			char* p = (char*)ptr;
			p += layout->struct_offset;
			return (T*)p;
		}
		
		void* NextChunk(void* ptr)
		{
			char* p = (char*)ptr;
			p += total_size_;
			return (void*)p;
		}

		template<typename T1>
		bool GetStruct(void* ptr, T1** t1)
		{
			*t1 = Get<T1>(ptr);
			if (!(*t1))return false;
			return true;
		}
		template<typename T1, typename T2>
		bool GetStruct(void* ptr, T1** t1,T2** t2)
		{
			*t1 = Get<T1>(ptr);
			if (!(*t1))return false;
			*t2 = Get<T2>(ptr);
			if (!(*t2))return false;
			return true;
		}
		template<typename T1, typename T2, typename T3>
		bool GetStruct(void* ptr, T1** t1, T2** t2,T3** t3)
		{
			*t1 = Get<T1>(ptr);
			if (!(*t1))return false;
			*t2 = Get<T2>(ptr);
			if (!(*t2))return false;
			*t3 = Get<T3>(ptr);
			if (!(*t3))return false;
			return true;
		}
		template<typename T1, typename T2, typename T3, typename T4>
		bool GetStruct(void* ptr, T1** t1, T2** t2, T3** t3,T4** t4)
		{
			*t1 = Get<T1>(ptr);
			if (!(*t1))return false;
			*t2 = Get<T2>(ptr);
			if (!(*t2))return false;
			*t3 = Get<T3>(ptr);
			if (!(*t3))return false;
			*t4 = Get<T4>(ptr);
			if (!(*t4))return false;
			return true;
		}
	};

	class ChunkMetaRegistration
	{
		static std::map<int, ChunkMeta*> ChunkMetaRegistration::type2chunkmeta_;
	public:
		static ChunkMeta*  CreateAndRegist(int chunk_type);
		static void Release(int chunk_type);
		static ChunkMeta* GetChunkMetaByType(int chunk_type);
	};

	class ChunkArray
	{
		ChunkMeta*		 meta_;
		BlockArrayPool*  array_;
	public:
		ChunkArray(ChunkMeta* meta);
		~ChunkArray();
		void Init(int chunk_size,int chunk_num);
		void* Create();
		void  Release(void* p);
		void  ReleaseByIdx(int idx);
		void* GetByIdx(int idx);
		void  Rearray();
		ChunkMeta* GetMeta() { return meta_; }
		void* GetChunkPtr();
		int  GetChunkSize();
		int  GetChunkUseNum();
	};

	

}