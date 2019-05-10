#pragma once

#include "zmemory.h"

class Buffer
{
	char* buf_;
	int   size_;

public:

	Buffer()
		:buf_((char*)0)
		, size_(0)
	{
	}

	~Buffer()
	{
		Release();
	}

	void Release()
	{
		if (buf_)
		{
			zm_free(buf_);
			buf_ = (char*)0;
			size_ = 0;
		}
	}

	int ReMalloc(int size)
	{
		Release();
		buf_ = (char*)zm_malloc(size);
		if (buf_)
		{
			size_ = size;
		}
		return size_;
	}

	int EnsureSize(int size)
	{
		if (size > size_)
		{
			ReMalloc(size);
		}
		return size_;
	}

	char* GetBuf()
	{
		return buf_;
	}

	int GetSize()
	{
		return size_;
	}
};