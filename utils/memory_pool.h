#pragma once

#include "zmemory.h"
#include <list>

//均为非线程安全对象池


class SimplePool
{
	std::list<void*> blocks_;
	int block_size_;
	int increase_num_;

	std::list<void*> raw_ptrs_;

	void _Increase()
	{
		char* p = (char*)zm_malloc(block_size_*increase_num_);
		raw_ptrs_.push_back(p);
		for (int i = 0; i < increase_num_; ++i)
		{
			Recycle(p);
			p += block_size_;
		}
	}
public:
	SimplePool()
	{
		block_size_ = 0;
		increase_num_ = 0;
	}
	~SimplePool()
	{
		for (std::list<void*>::iterator it = raw_ptrs_.begin(); it != raw_ptrs_.end(); ++it)
		{
			zm_free((*it));
		}
		raw_ptrs_.clear();
	}
	SimplePool(int block_size, int increase_num)
	{
		Init(block_size, increase_num);
	}
	void Init(int block_size, int increase_num)
	{
		block_size_ = block_size;
		increase_num_ = increase_num;
	}
	void* FetchBlock()
	{
		if (block_size_ == 0)return (void*)0;
		if (blocks_.size() == 0)
		{
			_Increase();
		}
		void* p = blocks_.front();
		blocks_.pop_front();
		return p;
	}
	void  Recycle(void* p)
	{
		blocks_.push_back(p);
	}
};


//简单连续块数组内存池，使用有要求，释放和分配重整都要在逻辑外部调用。
#define BLOCK_ARRAY_INCREASE_NUM 64
#define BLOCK_META_MARK_NOUSE 0
#define BLOCK_META_MARK_INUSE 1

#define BLOCK_META_FLAG_SIZE sizeof(int)   //chunk的头4个字节
class BlockArrayPool
{
	char* ptr_;
	int	  block_num;

	int	  block_increase_num_;
	int	  block_size_;
	int   block_use_num_;
	
	int   meta_info_size_;
	int   block_real_size_;

	void _MarkMetaFlag(char* p, int flag)
	{
		*((int*)p) = flag;
	}
public:

	BlockArrayPool()
	{
		ptr_ = (char*)0;
		block_num = 0;
		block_increase_num_ = 64;
		block_size_ = 0;
		block_use_num_ = 0;
		meta_info_size_ = BLOCK_META_FLAG_SIZE;
	}
	~BlockArrayPool()
	{
		if (ptr_)
		{
			zm_free(ptr_);
		}
	}
	void Init(int block_size, int increase_num = BLOCK_ARRAY_INCREASE_NUM)
	{
		block_size_ = block_size;
		block_increase_num_ = increase_num;
		block_real_size_ = block_size_ + meta_info_size_;
	}
	bool _IsFlag(char* p, int flag)
	{
		return (*((int*)p) == flag);
	}
	char* GetChunkPtr()
	{
		return (ptr_ + BLOCK_META_FLAG_SIZE);
	}
	int GetRealBlockSize()
	{
		return block_real_size_;
	}
	bool Increase(int num = BLOCK_ARRAY_INCREASE_NUM)
	{
		int malloc_size = (num + block_num) * (block_real_size_);
		char* p = (char*)zm_malloc(malloc_size);
		if (!p) return false;
		if(ptr_)memcpy(p, ptr_, block_real_size_ * block_use_num_);
		char* ptemp = ptr_;
		ptr_ = p;
		block_num += num;
		if(ptemp)zm_free(ptemp);
		return true;
	}
	void* GetBlockByIndex(int idx)
	{
		if (idx >= block_use_num_ || idx < 0)
		{
			return (void*)0;
		}
		return (ptr_ + BLOCK_META_FLAG_SIZE + block_real_size_*idx);
	}
	int GetBlockUseNum()
	{
		return block_use_num_;
	}
	void* AllocBlock()
	{
		if (block_use_num_ + 1 >= block_num)
		{
			Increase();
		}
		if (block_use_num_ + 1 >= block_num)
		{
			return (void*)0;
		}
		char* p = (ptr_ + block_real_size_*block_use_num_);
		block_use_num_ += 1;
		memset(p, 0x00, block_real_size_);
		_MarkMetaFlag(p, BLOCK_META_MARK_INUSE);
		return (void*)(p + BLOCK_META_FLAG_SIZE);
	}
	void FreeBlockByIndex(int idx)
	{
		if (idx >= block_use_num_ || idx < 0)
		{
			return;
		}
		_MarkMetaFlag(ptr_ + block_real_size_*idx, BLOCK_META_MARK_NOUSE);
	}
	void FreeBlock(void* p)
	{
		char* real_addr = ((char*)p - BLOCK_META_FLAG_SIZE);
		int addr = real_addr - ptr_;
		int n = addr % block_real_size_;
		if (n !=0 || block_real_size_ * block_num >= addr || addr < 0)
		{
			return;
		}
		_MarkMetaFlag((char*)real_addr, BLOCK_META_MARK_NOUSE);
	}
	void Rearray()
	{
		char* pb, *pe;
		int use_counter = 0;
		if (block_use_num_ == 0) return;
		pb = ptr_;
		pe = ptr_ + block_real_size_ * (block_use_num_-1);
		for (;; pb += block_real_size_)
		{
			if (_IsFlag(pb, BLOCK_META_MARK_INUSE))
			{
				use_counter += 1;
			}
			else
			{
				for (;; pe -= block_real_size_)
				{
					if (_IsFlag(pe, BLOCK_META_MARK_INUSE))
					{
						memcpy(pb, pe, block_real_size_);
						_MarkMetaFlag(pe, BLOCK_META_MARK_NOUSE);
						break;
					}
					if (pb == pe)
					{
						break;
					}
				}
			}
			block_use_num_ = use_counter;
			if (pb == pe)
			{
				return;
			}
		}
	}
};

