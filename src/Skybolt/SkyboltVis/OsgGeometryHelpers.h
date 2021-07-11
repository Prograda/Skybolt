#pragma once

#include <osg/Drawable>

namespace skybolt {
namespace vis {

class FixedBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
public:
	FixedBoundingBoxCallback(const osg::BoundingBox& boundingBox) : mBoundingBox(boundingBox) {}
	osg::BoundingBox computeBound(const osg::Drawable & drawable) const override
	{
		return mBoundingBox;
	}

private:
	osg::BoundingBox mBoundingBox;
};

inline osg::ref_ptr<FixedBoundingBoxCallback> createFixedBoundingBoxCallback(const osg::BoundingBox& boundingBox)
{
	return new FixedBoundingBoxCallback(boundingBox);
}

} // namespace vis
} // namespace skybolt
