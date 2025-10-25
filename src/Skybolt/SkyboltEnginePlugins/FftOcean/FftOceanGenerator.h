/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/Math/MathUtility.h>
#include "WaveSpectrumWindow.h"

#include <complex>
#include <vector>

#include <xsimd/xsimd.hpp>
#include <span>

namespace skybolt {
namespace vis {

struct FftOceanGeneratorConfig
{
	int seed;
	int textureSizePixels;
	float textureWorldSize;
	glm::vec2 windVelocity;
	float gravity;
	WaveSpectrumWindow waveSpectrumWindow;
	bool useMultipleCores = false;
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
	//! @param result is a pointer to a buffer of size mTextureSizePixels * mTextureSizePixels in which to store the calculated output
	void calculate(float time, const std::span<glm::vec3>& result);

	void setWindVelocity(const glm::vec2& windVelocity);

	glm::ivec2 getTextureSizePixels() const { return glm::ivec2(mTextureSizePixels, mTextureSizePixels); }
	glm::dvec2 getTextureWorldSize() const { return glm::dvec2(mTextureWorldSize, mTextureWorldSize); }

	typedef std::complex<float> complex_type;
	const std::vector<complex_type>& getHt0Image() const { return mHt0; } //!< Image has dimensions of (mTextureSizePixels, mTextureSizePixels)

private:
	Simd4 calcDispersion(Simd4 kx, Simd4 kz) const;
	float calcPhillips(int n, int m) const;
	float calcBruenton(int n, int m) const;
	void calcHt0();

	using aligned_complex_type = complex_type; //!< Must be alligned for simd/avx. Allocate with mufft_alloc to gaurantee alignment

	struct MufftArrayDeleter {
		void operator() (aligned_complex_type *res) const;
	};

	using aligned_complex_type_ptr = std::unique_ptr<aligned_complex_type[], MufftArrayDeleter>; //!< wrapped to gaurantee data is freed with mufft_free
	static aligned_complex_type_ptr allocateAlignedComplexType(size_t elementCount);

private:
	const int mTextureSizePixels;
	const float mTextureWorldSize;
	const float mOneOnTextureWorldSize;
	const float mGravity;
	const WaveSpectrumWindow mWaveSpectrumWindow;
	const bool mUseMultipleCores;

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
