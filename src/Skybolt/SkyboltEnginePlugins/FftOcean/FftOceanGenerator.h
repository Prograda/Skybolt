/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/MathUtility.h>

#include <boost/random/normal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <complex>
#include <vector>

#include <xsimd/xsimd.hpp>

namespace skybolt {
namespace vis {

struct FftOceanGeneratorConfig
{
	int seed;
	int textureSizePixels;
	float textureWorldSize;
	glm::vec2 windVelocity;
	glm::vec2 normalizedFrequencyRange; //!< (min, max) in range [0, 1]
	float gravity;
};

template <typename T>
struct Complex
{
	T real;
	T img;

	Complex() {}
	Complex(T real, T img) : real(real), img(img) {}
};

template <typename T>
inline Complex<T> operator*(const Complex<T>& a, const Complex<T>& b)
{
	return Complex<T>(a.real * b.real - a.img * b.img, a.real * b.img + b.real * a.img);
}

typedef xsimd::batch<float, 4> Simd4;
typedef std::complex<Simd4> complex_type_simd4;

class FftOceanGenerator
{
public:
	FftOceanGenerator(const FftOceanGeneratorConfig& config);
	~FftOceanGenerator();

	static float calcWindSpeedFromMaxWaveHeight(float maxWaveHeight, float g)
	{
		return std::sqrt(maxWaveHeight * g);
	}

	static float calcMaxWaveHeight(float windSpeed, float g)
	{
		// From paper
		return windSpeed * windSpeed / g;
	}

	//! Generates a vector displacement image of the wave field at given time
	void calculate(float time, std::vector<glm::vec3>& result);

	void setWindSpeed(float windSpeed);

	typedef std::complex<float> complex_type;

private:
	Simd4 calcDispersion(Simd4 kx, Simd4 kz) const;
	float calcPhillips(int n, int m) const;
	float calcBruenton(int n, int m) const;
	void calcHt0();
	complex_type generateRandomComplexGaussian();

	using aligned_complex_type = complex_type; //!< Must be alligned for simd/avx. Allocate with mufft_alloc to gaurantee alignment

	struct MufftArrayDeleter {
		void operator() (aligned_complex_type *res) const;
	};

	using aligned_complex_type_ptr = std::unique_ptr<aligned_complex_type[], MufftArrayDeleter>; //!< wrapped to gaurantee data is freed with mufft_free
	static aligned_complex_type_ptr allocateAlignedComplexType(size_t elementCount);

private:
	typedef boost::variate_generator<boost::mt19937, boost::random::normal_distribution<float> > RandomGenerator;
	RandomGenerator mRandom;
	glm::vec2 mNormalizedFrequencyRange; //!< (min, max) in range [0, 1]
	const int mTextureSizePixels;
	const float mTextureWorldSize;
	const float mOneOnTextureWorldSize;
	const float mGravity;
	glm::vec2 mWindVelocity;
	std::vector<complex_type> mHt0;
	std::vector<complex_type> mHt0Conj;

	aligned_complex_type_ptr mFftInputVertical;
	aligned_complex_type_ptr mFftInputHorizontal[2];
	aligned_complex_type_ptr mFftOutputVertical;
	aligned_complex_type_ptr mFftOutputHorizontal[2];

	std::unique_ptr<struct FftGeneratorData> mFftGeneratorData;
};

} // namespace vis
} // namespace skybolt
