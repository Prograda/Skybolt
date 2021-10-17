/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ParticleSystem.h"
#include <SkyboltSim/Physics/Atmosphere.h>
#include "SkyboltSim/Spatial/GreatCircle.h"
#include "SkyboltSim/Spatial/Positionable.h"
#include <SkyboltCommon/Random.h>
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace sim {

ParticleEmitter::ParticleEmitter(const Params& params) : mParams(params)
{
	mOrientation = getOrientationFromDirection(mParams.upDirection);
}

void ParticleEmitter::update(float dt, std::vector<Particle>& particles)
{
	// Calculate emitter velocity
	Vector3 position = mParams.positionable->getPosition();
	Vector3 emitterVelocity = mPrevPosition ?
		(position - *mPrevPosition) / double(dt) :
		Vector3();
	mPrevPosition = position;

	// Create particles
	mParticlesToEmit += mParams.emissionRate * mEmissionRateMultiplier * dt;
	int particleCount = int(mParticlesToEmit);
	if (particleCount > 0)
	{
		mParticlesToEmit -= particleCount;
		float dtSubstep = dt / particleCount;
		float timeOffset = 0;

		for (int i = 0; i < particleCount; ++i)
		{
			particles.push_back(createParticle(emitterVelocity, timeOffset));
			timeOffset += dtSubstep;
		}
	}
}

int ParticleEmitter::mNextParticleId = 0;

Particle ParticleEmitter::createParticle(const Vector3& emitterVelocity, float timeOffset) const
{
	Vector3 velocityRelEmitter = calculateParticleVelocityRelEmitter();
	Particle particle;
	particle.guid = mNextParticleId++;
	particle.position = mParams.positionable->getPosition() + velocityRelEmitter * double(timeOffset);
	particle.velocity = emitterVelocity +velocityRelEmitter;
	particle.radius = mParams.radius;
	particle.initialAlpha = 1.0f * mEmissionAlphaMultiplier;
	particle.alpha = particle.initialAlpha;
	return particle;
}

Vector3 ParticleEmitter::calculateParticleVelocityRelEmitter() const
{
	float azimuth = mParams.random->unitRand() * math::twoPiF();
	float elevation = mParams.random->rangedRand(float(mParams.elevationAngle.first), float(mParams.elevationAngle.last));
	float speed = mParams.random->rangedRand(float(mParams.speed.first), float(mParams.speed.last));
	
	float cosElevation = glm::cos(elevation);
	
	Vector3 velocity(
		speed * glm::sin(elevation),
		speed * glm::sin(azimuth) * cosElevation,
		speed * glm::cos(azimuth) * cosElevation
	);
	
	return mParams.positionable->getOrientation() * (mOrientation * velocity);
}

void ParticleKiller::update(float dt, std::vector<Particle>& particles)
{
	for (size_t i = 0; i < particles.size();)
	{
		particles[i].age += dt;
		if (particles[i].age > mLifetime)
		{
			std::swap(particles[i], particles[particles.size() - 1]);
			particles.pop_back();
		}
		else
		{
			++i;
		}
	}
}

void ParticleIntegrator::update(float dt, std::vector<Particle>& particles)
{
	static Atmosphere atmosphere = createEarthAtmosphere(); // TODO: get atmosphere specific to planet
	double density = particles.empty() ? 0.0 : atmosphere.getDensity(glm::length(particles.front().position) - earthRadius());

	double dtD = double(dt);
	double velocityDamping = std::exp(-mParams.atmosphericSlowdownFactor * density * dt);
	for (auto& particle : particles)
	{
		particle.velocity  *= velocityDamping;

		particle.position += particle.velocity * dtD;
		particle.radius += mParams.radiusLinearGrowthPerSecond * dt;
		particle.alpha = glm::mix(particle.initialAlpha, 0.0f, particle.age / mParams.lifetime);
	}
}

ParticleSystem::ParticleSystem(const Operations& operations, size_t reserveParticleCount) :
	mOperations(operations)
{
	mParticles.reserve(reserveParticleCount);
}

void ParticleSystem::update(float dt)
{
	for (const auto& operation : mOperations)
	{
		operation->update(dt, mParticles);
	}
}

} // namespace sim
} // namespace skybolt
