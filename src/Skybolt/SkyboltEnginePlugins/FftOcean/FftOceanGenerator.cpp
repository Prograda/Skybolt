/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "FftOceanGenerator.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <cxxtimer/cxxtimer.hpp>

#include <muFFT/fft.h>

using namespace skybolt;

namespace skybolt {
namespace vis {

struct FftGeneratorData
{
	mufft_plan_2d* verticalPlan;
	mufft_plan_2d* horizontalPlan[2];
};

FftOceanGenerator::aligned_complex_type_ptr FftOceanGenerator::allocateAlignedComplexType(size_t elementCount)
{
	return aligned_complex_type_ptr(
		reinterpret_cast<aligned_complex_type*>(mufft_alloc(elementCount * sizeof(aligned_complex_type))));
}

void FftOceanGenerator::MufftArrayDeleter::operator() (aligned_complex_type* p) const
{
	mufft_free(p);
}

// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.161.9102&rep=rep1&type=pdf
FftOceanGenerator::FftOceanGenerator(const FftOceanGeneratorConfig& config) :
	mRandom(boost::mt19937(config.seed), boost::random::normal_distribution<float>()),
	mNormalizedFrequencyRange(config.normalizedFrequencyRange),
	mTextureSizePixels(config.textureSizePixels),
	mTextureWorldSize(config.textureWorldSize),
	mOneOnTextureWorldSize(1.0f / mTextureWorldSize),
	mGravity(config.gravity),
	mWindVelocity(config.windVelocity),
	mFftGeneratorData(new FftGeneratorData)
{
	size_t size = config.textureSizePixels * config.textureSizePixels;
	mHt0.resize(size);
	mHt0Conj.resize(size);

	mFftInputVertical = allocateAlignedComplexType(size);
	mFftInputHorizontal[0] = allocateAlignedComplexType(size);
	mFftInputHorizontal[1] = allocateAlignedComplexType(size);

	mFftOutputVertical = allocateAlignedComplexType(size);
	mFftOutputHorizontal[0] = allocateAlignedComplexType(size);
	mFftOutputHorizontal[1] = allocateAlignedComplexType(size);

	calcHt0();

	mFftGeneratorData->verticalPlan = mufft_create_plan_2d_c2c(config.textureSizePixels, config.textureSizePixels, MUFFT_FORWARD, MUFFT_FLAG_CPU_ANY);

	for (int i = 0; i < 2; ++i)
	{
		mFftGeneratorData->horizontalPlan[i] = mufft_create_plan_2d_c2c(config.textureSizePixels, config.textureSizePixels, MUFFT_FORWARD, MUFFT_FLAG_CPU_ANY);
	}
}

FftOceanGenerator::~FftOceanGenerator()
{
	mufft_free_plan_2d(mFftGeneratorData->verticalPlan);
	for (int i = 0; i < 2; ++i)
	{
		mufft_free_plan_2d(mFftGeneratorData->horizontalPlan[i]);
	}
}

// Model the periodic dispersion relation, Section 3.2
Simd4 FftOceanGenerator::calcDispersion(Simd4 kx, Simd4 kz) const
{
	// Define base frequency at which to loop the dispersion
	static float w0 = 2 * math::piF() / 200.0f;

	// Equation 18
	return floor(sqrt(mGravity * sqrt(kx * kx + kz * kz)) / w0) * w0;
}

// Phillips Spectrum modulated by wind speed and direction
// Section 3.3, equation 23
float FftOceanGenerator::calcPhillips(int n, int m) const
{
	glm::vec2 normalizedCoord(2 * n - mTextureSizePixels, 2 * m - mTextureSizePixels);
	glm::vec2 k = math::piF() * normalizedCoord * mOneOnTextureWorldSize;

	float kLength = glm::length(k);

	if (kLength < 0.000001)
	{
		return 0.0;
	}

	float windSpeed = glm::length(mWindVelocity);

	// |k|^4
	float k_len2 = kLength * kLength;
	float k_len4 = k_len2 * k_len2;

	// |k dot w|^2
	float kw = glm::dot(k / kLength, mWindVelocity / windSpeed);
	kw = kw * kw;

	// L, L^2
	float Lsq = windSpeed * windSpeed / mGravity;
	Lsq = Lsq * Lsq;

	// Damping term for eqation 24
	float damping = 0.001f;
	float l2 = Lsq * damping * damping;

	// Equation 23, damped by eq. 24
	float A = 1.f;
	float val = A * expf(-1.f / (k_len2 * Lsq)) / k_len4 * kw;// *exp(-k_len2 * l2);

	float normalizedCoordLength = glm::length(normalizedCoord) * mOneOnTextureWorldSize;
	if (normalizedCoordLength < mNormalizedFrequencyRange.x || normalizedCoordLength > mNormalizedFrequencyRange.y)
	{
		return 0;
	}

	//#define DEBUG_1D_SINUSOID
#ifdef DEBUG_1D_SINUSOID
	return (std::abs(n - mTextureSizePixels / 2) == 8 && m == mTextureSizePixels / 2) ? 10 : 0;
#endif

#ifdef PRINT_SPECTRUM
	if (m == mTextureSizePixels / 2)
	{
		std::cout << val << std::endl;
	}
#endif
	return val;
}

float OMEGA = 0.84f; // sea state (inverse wave age)

const float cm = 0.23f; // Eq 59

const float km = 370.0f; // Eq 59

float sqr(float x)
{
	return x * x;
}

float omega(float k)
{
	return std::sqrt(9.81f * k * (1.0f + sqr(k / km))); // Eq 24
}

float omnispectrum = false;

float FftOceanGenerator::calcBruenton(int n, int m) const
{
	glm::vec2 normalizedCoord(2 * n - mTextureSizePixels, 2 * m - mTextureSizePixels);
	glm::vec2 k = math::piF() * normalizedCoord * mOneOnTextureWorldSize;

	float windSpeed = glm::length(mWindVelocity);

	float U10 = windSpeed;
	float Omega = OMEGA;

	// phase speed
	float kLength = std::sqrt(k.x * k.x + k.y * k.y);
	float c = omega(kLength) / kLength;

	if (kLength < 0.000001)
	{
		return 0.0;
	}

	// spectral peak
	float kp = 9.81f * sqr(Omega / U10); // after Eq 3
	float cp = omega(kp) / kp;

	// friction velocity
	float z0 = 3.7e-5f * sqr(U10) / 9.81f * pow(U10 / cp, 0.9f); // Eq 66
	float u_star = 0.41f * U10 / log(10.0f / z0); // Eq 60

	float Lpm = exp(-5.0f / 4.0f * sqr(kp / kLength)); // after Eq 3
	float gamma = Omega < 1.0f ? 1.7f : 1.7f + 6.0f * log(Omega); // after Eq 3 // log10 or log??
	float sigma = 0.08f * (1.0f + 4.0f / pow(Omega, 3.0f)); // after Eq 3
	float Gamma = exp(-1.0f / (2.0f * sqr(sigma)) * sqr(std::sqrt(kLength / kp) - 1.0f));
	float Jp = pow(gamma, Gamma); // Eq 3
	float Fp = Lpm * Jp * exp(-Omega / std::sqrt(10.0f) * (std::sqrt(kLength / kp) - 1.0f)); // Eq 32
	float alphap = 0.006f * std::sqrt(Omega); // Eq 34
	float Bl = 0.5f * alphap * cp / c * Fp; // Eq 31

	float alpham = 0.01f * (u_star < cm ? 1.0f + log(u_star / cm) : 1.0f + 3.0f * log(u_star / cm)); // Eq 44
	float Fm = exp(-0.25f * sqr(kLength / km - 1.0f)); // Eq 41
	float Bh = 0.5f * alpham * cm / c * Fm * Lpm; // Eq 40 (fixed)

	float A = 1.f;

	if (omnispectrum) {
		return A * (Bl + Bh) / (kLength * sqr(kLength)); // Eq 30
	}

	float a0 = log(2.0f) / 4.0f; float ap = 4.0; float am = 0.13f * u_star / cm; // Eq 59
	float Delta = tanh(a0 + ap * pow(c / cp, 2.5f) + am * pow(cm / c, 2.5f)); // Eq 57

	float phi = atan2(k.y, k.x);

	if (k.x < 0.0) {
		return 0.0;
	}
	else {
		Bl *= 2.0;
		Bh *= 2.0;
	}

	return A * (Bl + Bh) * (1.0f + Delta * cos(2.0f * phi)) / (2.0f * math::piF() * sqr(sqr(kLength))); // Eq 67
}

FftOceanGenerator::complex_type FftOceanGenerator::generateRandomComplexGaussian()
{
	// produces gaussian random draws with mean 0 and std dev 1
	float a = mRandom();
	float b = mRandom();
	return complex_type(a, b);
}

void FftOceanGenerator::calcHt0()
{
	float dk = 2.0f * math::piF() / mTextureWorldSize;

	for (int m = 0; m < mTextureSizePixels; ++m)
	{
		for (int n = 0; n < mTextureSizePixels; ++n)
		{
			int index = m * mTextureSizePixels + n;
			complex_type r = generateRandomComplexGaussian() / std::sqrt(2.f);
			mHt0[index] = r * std::sqrt(calcBruenton(n, m) / 2.0f) * dk;
			mHt0Conj[index] = std::conj(r * std::sqrt(calcBruenton(-n, -m) / 2.0f) * dk);
		}
	}
}

// x range: [-PI,PI]
float fast_sin(float x) {
	constexpr float PI = 3.14159265358f;
	constexpr float B = 4.0f / PI;
	constexpr float C = -4.0f / (PI * PI);
	constexpr float P = 0.225f;

	float y = B * x + C * x * (x < 0 ? -x : x);
	return P * (y * (y < 0 ? -y : y) - y) + y;
}

// x range: [-PI, PI]
float fast_cos(float x) {
	constexpr float PI = 3.14159265358f;
	constexpr float B = 4.0f / PI;
	constexpr float C = -4.0f / (PI * PI);
	constexpr float P = 0.225f;

	x = (x > 0) ? -x : x;
	x += PI / 2;

	return fast_sin(x);
}

/* wrap x -> [0,max) */
float wrapMax(float x, float max)
{
	/* integer math: `(max + x % max) % max` */
	return fmod(max + fmod(x, max), max);
}
/* wrap x -> [min,max) */
float wrapMinMax(float x, float min, float max)
{
	return min + wrapMax(x - min, max - min);
}

float wrapNegPiPosPi(float angle)
{
	return wrapMinMax(angle, -math::piF(), math::piF());
}

// Equation 26
static complex_type_simd4 ht(float t, Simd4 dispersion, const complex_type_simd4& ht0, const complex_type_simd4& ht0conj)
{
	// complex exponential is calculated by euler's identity
	// for faster computation
	Simd4 omegat = dispersion * t;
	Simd4 real = cos(omegat);
	Simd4 cmp = sin(omegat);

	complex_type_simd4 c0(real, cmp);
	complex_type_simd4 c1(real, -cmp);
	return ht0 * c0 + ht0conj * c1;
}

const Simd4 simd4Zero(0, 0, 0, 0);
inline Simd4 fromScalar(float v) { return Simd4(v, v, v, v); }

FftOceanGenerator::complex_type toSingle(complex_type_simd4 v, int i)
{
	return FftOceanGenerator::complex_type(v.real()[i], v.imag()[i]);
}

static float filterNan(float v, float valueIfNan)
{
	return std::isnan(v) ? valueIfNan : v;
}

void FftOceanGenerator::calculate(float t, std::vector<glm::vec3>& result)
{
	cxxtimer::Timer timer(true);

	result.resize(mTextureSizePixels * mTextureSizePixels);
	complex_type_simd4 htVertical, htHorizontalX, htHorizontalZ;

	// Prepare fft input
	for (int m = 0; m < mTextureSizePixels; ++m)
	{
		Simd4 kz = fromScalar(math::piF() * (2.0f * m - mTextureSizePixels) * mOneOnTextureWorldSize);
		for (int ns = 0; ns < mTextureSizePixels; ns+=4) // advance in blocks of 4 which are processed concurrently with simd4
		{
			Simd4 n(float(ns), float(ns + 1), float(ns + 2), float(ns + 3));
			Simd4 kx = math::piF() * (2 * n - (float)mTextureSizePixels) * mOneOnTextureWorldSize;
			Simd4 len = sqrt(kx * kx + kz * kz) + 0.0000001f; // add epsilon to prevent divide by zero
			int index = m * mTextureSizePixels + ns;

			// Pack 4 array items at a time into simd4
			complex_type_simd4 ht0(
				{mHt0[index + 0].real(), mHt0[index + 1].real(), mHt0[index + 2].real(), mHt0[index + 3].real()},
				{mHt0[index + 0].imag(), mHt0[index + 1].imag(), mHt0[index + 2].imag(), mHt0[index + 3].imag()});

			complex_type_simd4 ht0Conj(
				{mHt0Conj[index + 0].real(), mHt0Conj[index + 1].real(), mHt0Conj[index + 2].real(), mHt0Conj[index + 3].real()},
				{mHt0Conj[index + 0].imag(), mHt0Conj[index + 1].imag(), mHt0Conj[index + 2].imag(), mHt0Conj[index + 3].imag()});

			// Calculate dispersion with simd4 intrinsics
			htVertical = ht(t, calcDispersion(kx, kz), ht0, ht0Conj);

			// Unpack from simd4 to muFFT input
			htHorizontalX = htVertical * complex_type_simd4(simd4Zero, -kx / len);
			htHorizontalZ = htVertical * complex_type_simd4(simd4Zero, -kz / len);

			for (int i = 0; i < 4; ++i)
			{
				mFftInputVertical[index + i] = toSingle(htVertical, i);
				mFftInputHorizontal[0][index + i] = toSingle(htHorizontalX, i);
				mFftInputHorizontal[1][index + i] = toSingle(htHorizontalZ, i);
			}
		}
	}

	// Run FFT
	mufft_execute_plan_2d(mFftGeneratorData->verticalPlan, mFftOutputVertical.get(), mFftInputVertical.get());
	mufft_execute_plan_2d(mFftGeneratorData->horizontalPlan[0], mFftOutputHorizontal[0].get(), mFftInputHorizontal[0].get());
	mufft_execute_plan_2d(mFftGeneratorData->horizontalPlan[1], mFftOutputHorizontal[1].get(), mFftInputHorizontal[1].get());
	//std::cout << "time " << timer.count() << std::endl;

	// Output results to vector displacement image
	float lambda = 8.f; // Controls wave peak steepness
	float signs[] = { 1.0f, -1.0f };

	for (int m = 0; m < mTextureSizePixels; ++m)
	{
		for (int n = 0; n < mTextureSizePixels; ++n)
		{
			int index = m * mTextureSizePixels + n;
			int sign = (int)signs[(n + m) & 1];

			result[index].x = filterNan(mFftOutputHorizontal[0][index].real() * sign * lambda, 0.f);
			result[index].y = filterNan(mFftOutputHorizontal[1][index].real() * sign * lambda, 0.f);
			result[index].z = filterNan(mFftOutputVertical[index].real() * sign, 0.f);
		}
	}
}

void FftOceanGenerator::setWindSpeed(float windSpeed)
{
	mWindVelocity.x = windSpeed;
	calcHt0();
}

} // namespace vis
} // namespace skybolt
