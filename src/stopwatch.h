#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <iostream>
#include <time.h>

class Stopwatch
{
	timespec ts_launch, ts_current;

	static const uint32_t NANOSECS_PER_MICROSEC = (uint32_t)1e+3;
	static const uint32_t NANOSECS_PER_MILLISEC = (uint32_t)1e+6;
	static const uint32_t NANOSECS_PER_SEC		= (uint32_t)1e+9;

public:
	Stopwatch() { this->Reset(); }
	~Stopwatch() = default;

	void Reset()
	{
		UpdateTimespec(ts_launch);
	}

	void Update()
	{
		UpdateTimespec(ts_current);
	}

	uint64_t Nanoseconds()
	{
		return TimespecSubstractionNanoseconds(ts_current, ts_launch);
	}

	uint64_t Microseconds()
	{
		return TimespecSubstractionNanoseconds(ts_current, ts_launch) / NANOSECS_PER_MICROSEC;
	}

	uint64_t Milliseconds()
	{
		return TimespecSubstractionNanoseconds(ts_current, ts_launch) / NANOSECS_PER_MILLISEC;
	}

	uint64_t Seconds()
	{
		return TimespecSubstractionNanoseconds(ts_current, ts_launch) / NANOSECS_PER_SEC;
	}

private:
	uint64_t TimespecSubstractionNanoseconds(const timespec& ts1, const timespec& ts2)
	{
		return (ts_current.tv_sec - ts_launch.tv_sec) * NANOSECS_PER_SEC
			+ ((uint64_t)ts_current.tv_nsec - (uint64_t)ts_launch.tv_nsec);
	}
	void UpdateTimespec(timespec& ts)
	{
		int res = timespec_get(&ts, TIME_UTC);
	}
};

#endif