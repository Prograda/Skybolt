/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "ShapeSerializer.h"
#include <SkyboltCommon/Exception.h>
#include <LinearMath/btSerializer.h>
#include <btBulletWorldImporter.h>

namespace skybolt {
namespace sim {

void ShapeSerializer::save(const std::string& filepath, const btCollisionShape& shape)
{
	int maxSerializeBufferSize = 1024*1024*50;
	btDefaultSerializer serializer(maxSerializeBufferSize);
 
	serializer.startSerialization();
	shape.serializeSingleShape(&serializer);
	serializer.finishSerialization();
 
	FILE* file = fopen(filepath.c_str(),"wb");
	fwrite(serializer.getBufferPointer(), serializer.getCurrentBufferSize(), 1, file);
	fclose(file);
}

btCollisionShape* ShapeSerializer::load(const std::string& filepath)
{
	btBulletWorldImporter   fileLoader(0);
	if(fileLoader.loadFile(filepath.c_str()))
	{
		if (fileLoader.getNumCollisionShapes() > 0)
		{
			btCollisionShape* shape = fileLoader.getCollisionShapeByIndex(0);
			return shape;
		}
	}
	throw skybolt::Exception("Collision shape could not be loaded: " + filepath);
	return 0;
}

} // namespace skybolt
} // namespace sim
