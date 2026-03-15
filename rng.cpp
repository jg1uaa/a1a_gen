// SPDX-License-Identifier: WTFPL

#include <random>
#include <utility>
#include <cmath>
extern "C" {
#include "rng.h"
}

std::random_device rd;
std::mt19937 r(rd());

extern "C" void initialize_random_generator(void)
{
	// do nothing
}

extern "C" int random_value_int(int min, int max)
{
	if (min == max)
		return min;
	else if (min > max)
		std::swap(min, max);

	std::uniform_int_distribution<int> d(min, max);

	return d(r);
}

extern "C" double random_value_double(double min, double max)
{
	if (min == max)
		return min;
	else if (min > max)
		std::swap(min, max);

	std::uniform_real_distribution<double> d(min, std::nextafter(max, std::numeric_limits<double>::infinity()));

	return d(r);
}
