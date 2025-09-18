/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>
#include <FftOcean/FastRandomNormalDistribution.h>

using namespace skybolt::vis;

constexpr float epsilon = 1e-8f;

static float computeMean(const std::vector<float>& values)
{
    float sum = 0.0f;
    for (float v : values) sum += v;
    return sum / static_cast<float>(values.size());
}

static float computeStdDev(const std::vector<float>& values, float mean)
{
    float sum_sq_diff = 0.0f;
    for (float v : values) sum_sq_diff += (v - mean) * (v - mean);
    return std::sqrt(sum_sq_diff / static_cast<float>(values.size()));
}

static float chiSquaredUniform(const std::vector<float>& values, int num_bins)
{
    std::vector<int> counts(num_bins, 0);
    float bin_size = 1.0f / num_bins;
    for (float v : values)
    {
        int bin = static_cast<int>(v / bin_size);
        if (bin >= 0 && bin < num_bins) counts[bin]++;
    }
    float expected = static_cast<float>(values.size()) / num_bins;
    float chi2 = 0.0f;
    for (int count : counts)
    {
        chi2 += (count - expected) * (count - expected) / expected;
    }
    return chi2;
}

TEST_CASE("hashCoords produces deterministic output")
{
    CHECK(hashCoords(1, 2, 0) == hashCoords(1, 2, 0));
    CHECK(hashCoords(1, 2, 0) != hashCoords(2, 1, 0)); // Different inputs
    CHECK(hashCoords(1, 2, 0) != hashCoords(1, 2, 1)); // Different seed
}

TEST_CASE("hashToUniform produces values in (0, 1]")
{
    uint32_t h = hashCoords(1, 2, 0);
    float u = hashToUniform(h);
    CHECK(u > 0.0f);
    CHECK(u <= 1.0f);
}

TEST_CASE("hashToUniform produces uniform distribution")
{
    std::vector<float> values;
    for (int i = 0; i < 10000; ++i)
    {
        values.push_back(hashToUniform(hashCoords(i, 0, 0)));
    }
    // Chi-squared test for uniformity (10 bins)
    float chi2 = chiSquaredUniform(values, 10);
    // Critical value for chi-squared with 9 degrees of freedom at p=0.05 is ~16.92
    CHECK(chi2 < 16.92f);
}

TEST_CASE("uniformToNormal produces valid normal values")
{
    // Test with specific uniform inputs
    float u1 = 0.5f;
    float u2 = 0.5f;
    float n = uniformToStandardNormal(u1, u2);
    CHECK(std::isfinite(n)); // Should not be NaN or inf

    // Expected value for u1=0.5, u2=0.5 (approximate, based on Box-Muller)
    float expected = std::sqrt(-2.0f * std::log(0.5f)) * std::cos(2.0f * skybolt::math::piF() * 0.5f);
    CHECK(std::abs(n - expected) < epsilon);
}

TEST_CASE("deterministicNormalFast produces normal distribution")
{
    std::vector<float> values;
    const int num_samples = 100000;
    for (int i = 0; i < num_samples; ++i)
    {
        values.push_back(deterministicNormalFast(i, 0, 0));
    }

    float mean = computeMean(values);
    float stddev = computeStdDev(values, mean);
    CHECK(std::abs(mean) < 0.05f); // Mean should be close to 0
    CHECK(std::abs(stddev - 1.0f) < 0.05f); // Stddev should be close to 1

    // Check for no NaN or inf values
    for (float v : values)
    {
        REQUIRE(std::isfinite(v));
    }
}
