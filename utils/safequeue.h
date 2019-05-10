#pragma once

#include "mutex.h"
#include <list>

template<typename T>
class SafeQueue
{
	Mutex			mutex_;
	std::list<T>	cur_list_;

public:

	void Push(T& t)
	{
		mutex_.lock();
		cur_list_.push_back(t);
		mutex_.unlock();
	}

	bool Pop(T& t)
	{
		bool b = false;
		mutex_.lock();
		if (cur_list_.size() > 0)
		{
			t = cur_list_.front();
			cur_list_.pop_front();
			b = true;
		}
		mutex_.unlock();
		return b;
	}
};