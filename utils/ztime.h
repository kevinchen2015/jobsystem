#pragma once

#include <time.h>

float GetTimeNow()
{
	return (float)((long)clock()) / ((long)CLOCKS_PER_SEC);
}

struct TimeProfiler
{
	float start_time;
	float end_time;
	float time_cost;
};

void TimeProfilerStart(TimeProfiler* profiler)
{
	profiler->start_time = GetTimeNow();
}

void TimeProfilerEnd(TimeProfiler* profiler)
{
	profiler->end_time = GetTimeNow();
	profiler->time_cost = profiler->end_time - profiler->start_time;
}