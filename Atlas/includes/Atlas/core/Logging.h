#pragma once

#include <cassert>

#define CHECK(condition) assert(condition)

#ifdef _DEBUG

#define DCHECK(condition)	CHECK(condition)

#else

#define DCHECK(condition)

#endif