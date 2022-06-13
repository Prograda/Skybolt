/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "WaterStateSet.h"
#include "SkyboltVis/OsgBox2.h"
#include "SkyboltVis/OsgImageHelpers.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/OsgStateSetHelpers.h"
#include <osg/TextureBuffer>
#include <osgDB/ReadFile>

#include <assert.h>

const int wakeParamsImageSize = 2048;
const int hashCellsX = 128; // TODO: ensure this matches the shader
const osg::Vec2f hasCellWorldSize(100, 100); // TODO: ensure this matches the shader

namespace skybolt {
namespace vis {

WaterStateSet::WaterStateSet(const WaterStateSetConfig& config)
{
	osg::Uniform* heightMapTexCoordScales = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "heightMapTexCoordScales", config.waveTextureCount);
	for (int i = 0; i < config.waveTextureCount; ++i)
	{
		// TODO: replace 10000 with Scene::mWrappedNoisePeriod
		osg::Vec2f size(10000 / config.waveHeightMapWorldSizes[i], 10000 / config.waveHeightMapWorldSizes[i]);
		heightMapTexCoordScales->setElement(i, size);
	}

	osg::ref_ptr<osg::TextureBuffer> wakeHashMapTexture;
	{
		mWakeHashMapImage = new osg::Image;
		mWakeHashMapImage->allocateImage(hashCellsX*hashCellsX * 2, 1, 1, GL_RED, GL_FLOAT);

		wakeHashMapTexture = new osg::TextureBuffer;
		wakeHashMapTexture->setImage(mWakeHashMapImage);
		wakeHashMapTexture->setInternalFormat(GL_R32F);
	}

	osg::ref_ptr<osg::TextureBuffer> wakeParamsTexture;
	{
		mWakeParamsImage = new osg::Image;
		mWakeParamsImage->allocateImage(wakeParamsImageSize, 1, 1, GL_RGBA, GL_FLOAT); // TODO: use GL_RGB

		wakeParamsTexture = new osg::TextureBuffer;
		wakeParamsTexture->setImage(mWakeParamsImage);
		wakeParamsTexture->setInternalFormat(GL_RGBA32F_ARB);
	}

	addUniform(heightMapTexCoordScales);
	osg::Texture2D* foamTexture = new osg::Texture2D(osgDB::readImageFile("Environment/Foam.jpg"));
	foamTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	foamTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

	int i = 0;
	if (config.waveTextureCount == 1)
	{
		setTextureAttributeAndModes(i++, config.waveHeightTexture[0], osg::StateAttribute::ON);
		setTextureAttributeAndModes(i++, config.waveNormalTexture[0], osg::StateAttribute::ON);
		mFirstFoamMaskTextureIndex = i;
		setTextureAttributeAndModes(i++, config.waveFoamMaskTexture[0], osg::StateAttribute::ON);
	}
	else
	{
		assert(config.waveTextureCount == 2);

		setTextureAttributeAndModes(i++, config.waveHeightTexture[0], osg::StateAttribute::ON);
		setTextureAttributeAndModes(i++, config.waveHeightTexture[1], osg::StateAttribute::ON);
		setTextureAttributeAndModes(i++, config.waveNormalTexture[0], osg::StateAttribute::ON);
		setTextureAttributeAndModes(i++, config.waveNormalTexture[1], osg::StateAttribute::ON);
		mFirstFoamMaskTextureIndex = i;
		setTextureAttributeAndModes(i++, config.waveFoamMaskTexture[0], osg::StateAttribute::ON);
		setTextureAttributeAndModes(i++, config.waveFoamMaskTexture[1], osg::StateAttribute::ON);
	}
	setTextureAttributeAndModes(i++, foamTexture, osg::StateAttribute::ON);
	setTextureAttribute(i++, wakeHashMapTexture, osg::StateAttribute::ON);
	setTextureAttribute(i++, wakeParamsTexture, osg::StateAttribute::ON);

	i = 0;
	addUniform(createArrayOfUniformSampler2d("heightSamplers", i, config.waveTextureCount));
	i += config.waveTextureCount;
	addUniform(createArrayOfUniformSampler2d("normalSamplers", i, config.waveTextureCount));
	i += config.waveTextureCount;
	addUniform(createArrayOfUniformSampler2d("foamMaskSamplers", i, config.waveTextureCount));
	i += config.waveTextureCount;
	addUniform(createUniformSampler2d("foamSampler", i++));
	addUniform(createUniformSamplerTbo("wakeHashMapTexture", i++));
	addUniform(createUniformSamplerTbo("wakeParamsTexture", i++));
}
struct WakeSegment
{
	osg::Vec3f start;
	osg::Vec3f end;
};

static Box2f getBounds(const WakeSegment& segment)
{
	Box2f startBox(
		osg::Vec2f(segment.start.x() - segment.start.z(), segment.start.y() - std::abs(segment.start.z())),
		osg::Vec2f(segment.start.x() + segment.start.z(), segment.start.y() + std::abs(segment.start.z())));

	Box2f endBox(
		osg::Vec2f(segment.end.x() - segment.end.z(), segment.end.y() - std::abs(segment.end.z())),
		osg::Vec2f(segment.end.x() + segment.end.z(), segment.end.y() + std::abs(segment.end.z())));

	Box2f box;
	box.merge(startBox);
	box.merge(endBox);
	return box;
}

static osg::Vec2i calcHashCellCoord(const osg::Vec2f& point)
{
	osg::Vec2i index;
	osg::Vec2f indexF = math::componentWiseDivide(point, hasCellWorldSize);
	return osg::Vec2i(
		int(floorf(indexF.x())),
		int(floorf(indexF.y())));
}

int modPositive(int a, int b)
{
	return (a%b + b) % b;
}

int calcHashCellIndexFromCoord(const osg::Vec2i& c)
{
	return modPositive(c.x(), hashCellsX) + modPositive(c.y(), hashCellsX) * hashCellsX;
}

std::string toString(const std::vector<int>& ids)
{
	std::string r;
	for (int id : ids)
	{
		r += std::to_string(id) + "_";
	}
	return r;
}

void WaterStateSet::setWakes(const std::vector<Wake>& wakes)
{
	float* hashMapData = (float*)mWakeHashMapImage->data();
	memset(hashMapData, 0, hashCellsX*hashCellsX * 2 * sizeof(float));

	osg::Vec4f* paramsData = (osg::Vec4f*)mWakeParamsImage->data();

	// Build map of hashes to segment arrays
	typedef std::map<int, std::vector<int>> WakeSegmentsMap;
	WakeSegmentsMap segmentsMap;

	std::vector<WakeSegment> segments;

	for (const Wake& wake : wakes)
	{
		int lastPointIndex = (int)wake.points.size() - 1;
		for (int i = 0; i < lastPointIndex; ++i)
		{
			int segmentIndex = segments.size();
			WakeSegment segment = { wake.points[i], wake.points[i + 1] };
			segments.push_back(segment);

			const Box2f& box = getBounds(segment);

			osg::Vec2i c0 = calcHashCellCoord(box.minimum);
			osg::Vec2i c1 = calcHashCellCoord(box.maximum);

			for (int y = c0.y(); y <= c1.y(); ++y)
			{
				for (int x = c0.x(); x <= c1.x(); ++x)
				{
					int index = calcHashCellIndexFromCoord(osg::Vec2i(x, y));
					segmentsMap[index].push_back(segmentIndex);
				}
			}
		}
	}

	// Use segmentsMap to build buffers to send to GPU
	typedef std::pair<int, int> Range;
	std::map<std::string, Range> segmentArrays; // Map segment array name to params buffer range. This allows common segment arrays (having the same name) to be reused.

	int nextParamsBufferIndex = 0;
	for (const auto& item : segmentsMap)
	{
		const std::vector<int>& segmentArray = item.second;
		std::string arrayName = toString(segmentArray);

		Range paramsBufferRange = { nextParamsBufferIndex , nextParamsBufferIndex + segmentArray.size() * 2 };
		paramsBufferRange.second = std::min(paramsBufferRange.second, wakeParamsImageSize);
		auto r = segmentArrays.insert(std::make_pair(arrayName, paramsBufferRange));
		if (r.second) // if inserted
		{
			for (int segmentIndex : segmentArray)
			{
				const WakeSegment& segment = segments[segmentIndex];

				if (nextParamsBufferIndex + 2 > wakeParamsImageSize)
				{
					break;
				}

				paramsData[nextParamsBufferIndex++] = osg::Vec4f(segment.start, 0);
				paramsData[nextParamsBufferIndex++] = osg::Vec4f(segment.end, 0);
			}
		}
		else
		{
			paramsBufferRange = r.first->second;
		}

		hashMapData[item.first * 2] = float(paramsBufferRange.first) + 0.5f; // Add 0.5 to avoid imprecession in conversion from float to int in the shader
		hashMapData[item.first * 2 + 1] = float(paramsBufferRange.second) + 0.5f;
	}

	mWakeHashMapImage->dirty();
	mWakeParamsImage->dirty();
}

void WaterStateSet::setFoamTexture(int index, osg::ref_ptr<osg::Texture2D> texture)
{
	setTextureAttributeAndModes(mFirstFoamMaskTextureIndex + index, texture, osg::StateAttribute::ON);
}
 
} // namespace vis
} // namespace skybolt
