#pragma once

#include <cassert>

#define VERIFY_DEBUG_BREAK 0

#define verify(expr)\
[value = (expr)]() { \
	if(!value && VERIFY_DEBUG_BREAK) \
		__debugbreak(); \
	return value; \
}() 
