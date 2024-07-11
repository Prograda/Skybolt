/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


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

void configureDrawable(osg::Drawable& drawable);

} // namespace vis
} // namespace skybolt
