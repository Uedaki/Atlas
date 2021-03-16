#include "atlas/core/Random.h"

using namespace atlas;

Float atlas::random()
{
	thread_local static uint32_t x = 123456789;
	thread_local static uint32_t y = 362436069;
	thread_local static uint32_t z = 521288629;
	thread_local static uint32_t w = 88675123;
	uint32_t t;
	t = x ^ (x << 11);
	x = y; y = z; z = w;
	w = w ^ (w >> 19) ^ (t ^ (t >> 8));
	return (static_cast<float>(w & 0xffffff) / 16777216.0f);
}