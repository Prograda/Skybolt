/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ModelPreparer.h"
#include <SkyboltVis/OsgGeometryHelpers.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osgUtil/TangentSpaceGenerator>

namespace skybolt {
namespace vis {

ModelPreparer::ModelPreparer(const ModelPreparerConfig& config) :
	NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN),
	mGenerateTangents(config.generateTangents)
{
}
 
void ModelPreparer::apply(osg::Node &node)
{
 	traverse(node);
}

void ModelPreparer::apply(osg::Geode &geode)
{
	unsigned int numGeoms = geode.getNumDrawables();
	for(unsigned int geodeIdx = 0; geodeIdx < numGeoms; geodeIdx++)
	{
		osg::Geometry *geom = geode.getDrawable(geodeIdx)->asGeometry();
		if (geom)
		{
			vis::configureDrawable(*geom);

			if (mGenerateTangents)
			{
				osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator();
				tsg->generate(geom, 0);
				geom->setTexCoordArray(1, tsg->getTangentArray());
			}
		} 
	}
}    

} // namespace vis
} // namespace skybolt
