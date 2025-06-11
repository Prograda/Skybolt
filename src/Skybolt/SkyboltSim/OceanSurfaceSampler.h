#pragma once

#include "SkyboltSim/SimMath.h"
#include "SkyboltSim/Spatial/LatLon.h"

namespace skybolt::sim {

class OceanSurfaceSampler
{
public:
	virtual ~OceanSurfaceSampler() = default;

	virtual double calcOceanAltitude(const skybolt::sim::LatLon& position, double filterRadius) const = 0;
	virtual skybolt::sim::Vector3 calcOceanNormalNed(const skybolt::sim::LatLon& position, double filterRadius) const = 0;
};

class PlanarOceanSurfaceSampler : public OceanSurfaceSampler
{
public:
	PlanarOceanSurfaceSampler(double surfaceAltitude = 0) : mSurfaceAltitude(surfaceAltitude) {}
	~PlanarOceanSurfaceSampler() override = default;
	
	double calcOceanAltitude(const skybolt::sim::LatLon& position, double filterRadius) const override
	{
		return mSurfaceAltitude;
	}
	
	skybolt::sim::Vector3 calcOceanNormalNed(const skybolt::sim::LatLon& position, double filterRadius) const override
	{
		return skybolt::sim::Vector3(0,0,-1);
	}

private:
	double mSurfaceAltitude;
};

} // namespace skybolt::sim