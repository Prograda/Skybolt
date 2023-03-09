/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include <SkyboltCommon/Range.h>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace skybolt {

class Random;

namespace sim {

struct Particle
{
	int guid; //!< ID of particle, semi-unique across all particle systems. May repeat after numeric limit is reached.
	Vector3 position;
	Vector3 velocity;
	float radius;
	float age;
	float initialAlpha;
	float alpha;
	float temperatureDegreesCelcius;
};

class ParticleSystemOperation
{
public:
	virtual ~ParticleSystemOperation() {}
	virtual void update(float dt, std::vector<Particle>& particles) = 0;
};

using NearestPlanetProvider = std::function<sim::Entity*(const sim::Vector3& position)>;

class ParticleEmitter : public ParticleSystemOperation
{
public:
	struct Params
	{
		PositionablePtr positionable;
		float emissionRate;
		float radius;
		Vector3 upDirection;
		DoubleRangeInclusive speed;
		DoubleRangeInclusive elevationAngle;
		float temperatureDegreesCelcius;
		float zeroAtmosphericDensityAlpha;
		float earthSeaLevelAtmosphericDensityAlpha;

		std::shared_ptr<Random> random;
		NearestPlanetProvider nearestPlanetProvider;
	};

	ParticleEmitter(const Params& params);
	~ParticleEmitter() override = default;

	void update(float dt, std::vector<Particle>& particles) override;

	void setEmissionRateMultiplier(float emissionRateMultiplier)
	{
		mEmissionRateMultiplier = emissionRateMultiplier; 
	}

	void setEmissionAlphaMultiplier(float emissionAlphaMultiplier)
	{
		mEmissionAlphaMultiplier = emissionAlphaMultiplier;
	}

	virtual Particle createParticle(const Vector3& emitterVelocity, float timeOffset) const;

private:
	sim::Entity* getNearestPlanet() const;
	float getAtmosphericDensity() const; // kg / m^3

	Vector3 calculateParticleVelocityRelEmitter() const;

private:
	const Params mParams;
	Matrix3 mOrientation;
	float mParticlesToEmit = 0;
	float mEmissionRateMultiplier = 1.0;
	float mEmissionAlphaMultiplier = 1.0;
	std::optional<Vector3> mPrevPosition;
	static int mNextParticleId;
};

class ParticleKiller : public ParticleSystemOperation
{
public:
	ParticleKiller(float lifetime) : mLifetime(lifetime) {}
	~ParticleKiller() override = default;

	void update(float dt, std::vector<Particle>& particles) override;

private:
	const float mLifetime;
};

class ParticleIntegrator : public ParticleSystemOperation
{
public:
	struct Params
	{
		float radiusLinearGrowthPerSecond;
		float lifetime;
		float atmosphericSlowdownFactor;
		std::optional<float> heatTransferCoefficent;
		NearestPlanetProvider nearestPlanetProvider;
	};

	ParticleIntegrator(const Params& params);
	void update(float dt, std::vector<Particle>& particles) override;

private:
	Params mParams;
	std::optional<glm::dmat4> mPrevPlanetTransform;
};

class ParticleSystem
{
public:
	using Operations = std::vector<std::shared_ptr<ParticleSystemOperation>>;
	
	ParticleSystem(const Operations& operations, size_t reserveParticleCount = 5000);

	void update(float dt);

	const std::vector<Particle>& getParticles() const { return mParticles; }

	template <class T>
	std::shared_ptr<T> getOperationOfType()
	{
		for (const auto& operation : mOperations)
		{
			if (auto r = std::dynamic_pointer_cast<T>(operation); r)
			{
				return r;
			}
		}
		return nullptr;
	}

private:
	Operations mOperations;
	std::vector<Particle> mParticles;
};

} // namespace sim
} // namespace skybolt
