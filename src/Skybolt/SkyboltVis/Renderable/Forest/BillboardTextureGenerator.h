/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Node>
#include <osg/Texture2D>
#include <osgViewer/Viewer>

namespace skybolt {
namespace vis {

struct BillboardTextureCamera
{
	osg::ref_ptr<osg::Camera> camera;
	osg::ref_ptr<osg::Image> albedoOutput;
	osg::ref_ptr<osg::Image> normalOutput;
};

enum BillboardTextureFitMode
{
	BillboardTextureFitBest,
	BillboardTextureFitWidth,
	BillboardTextureFitHeight
};

BillboardTextureCamera createBillboardTextureCamera(const osg::ref_ptr<osg::Node>& model, const osg::Vec3f& cameraForwardDir,  int width, int height, BillboardTextureFitMode fitMode);

void growEdges(osg::Image& image, int iterations = 100);

void normalizeNormalMap(osg::Image& image);

} // namespace vis
} // namespace skybolt
