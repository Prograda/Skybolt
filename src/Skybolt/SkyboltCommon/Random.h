/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "boost/random.hpp"

namespace skybolt {

class Random
{
	typedef boost::minstd_rand base_generator_type;
public:
	Random(int seed) :
		generator(seed),
		rnd(generator, boost::uniform_int<>(0, RAND_MAX))
	{
	}

	int getInt(int minValue, int maxValue)
	{
		return rnd() % (1 + maxValue - minValue) + minValue;
	}

	float unitRand()
	{
		return (float)rnd()/RAND_MAX;
	}

	float rangedRand(float minValue, float maxValue)
	{
		return minValue + unitRand() * (maxValue - minValue);
	}

private:
	base_generator_type generator;
	boost::variate_generator<base_generator_type&, boost::uniform_int<> > rnd;
};

} // namespace skybolt