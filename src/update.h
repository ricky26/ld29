#ifndef  _UPDATE_H_
#define  _UPDATE_H_

#include <cstdint>

struct Update
{
	float dt;

	uint64_t ticksLast;
	uint64_t ticksNow;
	
	uint64_t tickFrequency;
};

#endif //_UPDATE_H_
