/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ModelPreparer.h"
#include <osg/Geode>
#include <osg/Geometry>

namespace skybolt {
namespace vis {

ModelPreparer::ModelPreparer() :
	NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN)
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
			geom->setUseDisplayList(false); 
			geom->setUseVertexBufferObjects(true); 
			geom->setUseVertexArrayObject(true);
		} 
	}
}    

} // namespace vis
} // namespace skybolt
