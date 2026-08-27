#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Physics/Atmosphere.h>

namespace skybolt::sim {

struct ValueAtWavelength
{
	double wavelengthNanometers;
	double value;
};

struct AtmosphereComponent : public skybolt::sim::Component
{
public:
	//! Scattering coefficient in units of m-1, at 440nm wavelength.
	//! The scattering coefficient at other wavelengths can be calculated from this using:
	//! `beta = beta440 * pow(lambda / 440nm, -4)`
	double rayleighScatteringCoeffAt440nm = 3.31e-5;
	double rayleighScaleHeight = 8000.0;

	double mieAngstromAlpha = 0.0;
	double mieExtinctionCoeff = 4.44e-6; //!< Units of m-1
	double mieSingleScatteringAlbedo = 0.9;
	double mieScaleHeight = 1200.0;
	double miePhaseFunctionG = 0.8;

	bool useEarthOzone = true;

	Atmosphere atmosphere = createEarthAtmosphere(); // TODO: use planet specific atmospheric parameters
};

SKYBOLT_REFLECT(AtmosphereComponent)
{
	registry.type<AtmosphereComponent>("AtmosphereComponent")
		.superType<Component>()
		.property("rayleighScatteringCoeffAt440nm", &AtmosphereComponent::rayleighScatteringCoeffAt440nm)
		.property("rayleighScaleHeight", &AtmosphereComponent::rayleighScaleHeight)
		.property("mieAngstromAlpha", &AtmosphereComponent::mieAngstromAlpha)
		.property("mieExtinctionCoeff", &AtmosphereComponent::mieExtinctionCoeff)
		.property("mieSingleScatteringAlbedo", &AtmosphereComponent::mieSingleScatteringAlbedo)
		.property("mieScaleHeight", &AtmosphereComponent::mieScaleHeight)
		.property("miePhaseFunctionG", &AtmosphereComponent::miePhaseFunctionG);
};

} // namespace skybolt::sim