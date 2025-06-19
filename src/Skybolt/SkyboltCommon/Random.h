/* Copyright Matthew Reid
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <random>

namespace skybolt {

class Random
{
public:
	Random(std::uint32_t seed)
	{
		// Seed with a sequence of prime numbers to achieve good initial state. 
		std::vector<std::uint32_t> seedData = {seed, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157};
		std::seed_seq seedSeq(seedData.begin(), seedData.end());
		generator.seed(seedSeq);

		// Warm up the random number generator by discarding samples to improve randomness.
		generator.discard(10000);
	}

	int getInt(int minValue, int maxValue)
	{
		std::uniform_int_distribution<int> dist(minValue, maxValue);
		return dist(generator);
	}

	float unitRand()
	{
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		return dist(generator);
	}

	float rangedRand(float minValue, float maxValue)
	{
		std::uniform_real_distribution<float> dist(minValue, maxValue);
		return dist(generator);
	}

	std::mt19937& getGenerator() { return generator; }

private:
	std::mt19937 generator;
};

} // namespace skybolt