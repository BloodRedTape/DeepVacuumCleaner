#pragma once

#include <random>

template<typename Type>
Type GetRandom(Type min, Type max);

template<>
inline float GetRandom(float min, float max) {
	float e = min + (rand() / float(RAND_MAX)) * (max - min);

	assert(!std::isinf(e) && !std::isnan(e));

	return e;
}