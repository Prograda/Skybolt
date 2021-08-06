/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "RunwaysBatch.h"
#include "OsgGeometryHelpers.h"
#include "OsgImageHelpers.h"
#include "OsgStateSetHelpers.h"

#include <SkyboltCommon/Math/MathUtility.h>

#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgText/Font>

#include <boost/log/trivial.hpp>
#include <assert.h>
#include <cctype>

const float runwayHeightAboveTerrain = 0.0f;
const float runwayTextureAspectRatio = 0.5f;

using namespace skybolt;

namespace skybolt {
namespace vis {

static osg::Vec3f toVec3(const osg::Vec2f& v, float z)
{
	return osg::Vec3f(v.x(), v.y(), z);
}

struct FontAtlas
{
	osg::ref_ptr<osg::Texture2D> texture;
	std::map<char, Box2f> characterBounds;
};

static osg::Vec2f flipV(const osg::Vec2f& v)
{
	return osg::Vec2f(v.x(), 1.0 - v.y());
}

static FontAtlas createFontAtlas(const std::string& characters)
{
	static osg::ref_ptr<osgText::Font> font = osgText::readRefFontFile("Fonts/ICAORWYID.ttf");
	
	FontAtlas atlas;
	atlas.texture = nullptr;

	for (const char& character : characters)
	{
		osgText::Glyph* glyph = font->getGlyph(osgText::FontResolution(32, 32), character);

		const osgText::Glyph::TextureInfo* info = glyph->getOrCreateTextureInfo(osgText::GREYSCALE);
		assert(!atlas.texture || atlas.texture == info->texture); // All glyphs should use the same texture
		atlas.texture = info->texture;
		atlas.characterBounds[character] = Box2f(info->minTexCoord, info->maxTexCoord);
	}
	return atlas;
}

struct MeshBuffers
{
	osg::Vec3Array* position = new osg::Vec3Array();
	osg::Vec3Array* normal = new osg::Vec3Array();
	osg::Vec2Array* uv = new osg::Vec2Array();
	osg::UIntArray* index = new osg::UIntArray();
};

static osg::Geometry* createGeometry(const MeshBuffers& buffers)
{
	osg::Geometry *geometry = new osg::Geometry();

	geometry->setVertexArray(buffers.position);
	geometry->setNormalArray(buffers.normal);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setTexCoordArray(0, buffers.uv);
	configureDrawable(*geometry);

	geometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, buffers.index->size(), (GLuint*)buffers.index->getDataPointer()));
	geometry->setComputeBoundingBoxCallback(createFixedBoundingBoxCallback(osg::BoundingBox())); // TODO: set bounds
	return geometry;
}

static void addTriquadIndices(osg::UIntArray* indices, int firstVertIndex)
{
	indices->push_back(firstVertIndex);
	indices->push_back(firstVertIndex + 1);
	indices->push_back(firstVertIndex + 2);
	indices->push_back(firstVertIndex + 1);
	indices->push_back(firstVertIndex + 3);
	indices->push_back(firstVertIndex + 2);
}

class FlatShapesGenerator
{
public:
	FlatShapesGenerator(MeshBuffers& buffers, const osg::Matrix& transform) :
		mBuffers(buffers),
		mTransform(transform)
	{
	}

	void addQuad(const osg::Vec2f& minPositionBound, const osg::Vec2f& maxPositionBound, const osg::Vec2f& minUvBound, const osg::Vec2f& maxUvBound)
	{
		size_t firstVertIndex = mBuffers.position->size();

		addPositionVertex(minPositionBound);
		addPositionVertex(osg::Vec2f(minPositionBound.x(), maxPositionBound.y()));
		addPositionVertex(osg::Vec2f(maxPositionBound.x(), minPositionBound.y()));
		addPositionVertex(maxPositionBound);

		for (int i = 0; i < 4; ++i)
		{
			mBuffers.normal->push_back(osg::Vec3(0, 0, -1));
		}

		mBuffers.uv->push_back(minUvBound);
		mBuffers.uv->push_back(osg::Vec2f(maxUvBound.x(), minUvBound.y()));
		mBuffers.uv->push_back(osg::Vec2f(minUvBound.x(), maxUvBound.y()));
		mBuffers.uv->push_back(maxUvBound);

		addTriquadIndices(mBuffers.index, firstVertIndex);
	}

	void setTransform(const osg::Matrix& transform)
	{
		mTransform = transform;
	}

private:
	void addPositionVertex(const osg::Vec2f& p)
	{
		mBuffers.position->push_back(transformPosition(p));
	}

	osg::Vec3f transformPosition(const osg::Vec2f& p) const
	{
		osg::Vec3f pos(p.x(), p.y(), 0.0);
		return pos * mTransform;
	}

private:
	MeshBuffers& mBuffers;
	osg::Matrix mTransform;
};

inline osg::Vec2f toVec2f(const osg::Vec3f& v)
{
	return osg::Vec2f(v.x(), v.y());
}

static void createRunwaySurface(const Runway& runway, const osg::Matrix& transform, MeshBuffers& buffers)
{
	osg::Vec2f displacement = toVec2f(runway.endPoint - runway.startPoint);
	float length = displacement.length();
	float tFinal = runwayTextureAspectRatio * length / runway.width;
	float halfWidth = runway.width * 0.5f;
	float halfLength = length * 0.5f;

	FlatShapesGenerator generator(buffers, transform);
	generator.addQuad(osg::Vec2f(-halfLength, -halfWidth), osg::Vec2f(halfLength, halfWidth), osg::Vec2f(0, 0), osg::Vec2f(1, tFinal));
}

static std::pair<std::string, std::string> splitRunwayNumberAndLetter(const std::string& runwayMarking)
{
	if (runwayMarking.empty())
	{
		return {"", ""};
	}
	char suffix = runwayMarking.back();
	if (suffix == 'L' || suffix == 'C' || suffix == 'R')
	{
		std::string number = runwayMarking;
		number.pop_back();
		return { number, std::string(1, suffix) };
	}
	return { runwayMarking, "" };
}

struct RunwayEndMarkingsConfig
{
	float halfWidth;
	float halfLength;
	osg::Vec2f whiteUv;
	std::string markingText;
	FlatShapesGenerator* generator;
	const FontAtlas* atlas;
};

static void createMarkingsForRunwayEnd(const RunwayEndMarkingsConfig& c)
{
	// Marking sizes from FAA AC 150/5340-1K
	// http://128.173.204.63/courses/cee4674/cee4674_pub/airport_markings_signs_2015_rfs.pdf
	float sideStripeWidth = 1;

	float maxCharacterWidth = 6;
	float textHeight = 18;
	float interLetterGap = 5.0f;

	float markingLetterStartDistance = 63;
	float markingNumberStartDistance = 87;

	float touchdownStripesStartDistance = 150;
	float touchdownStripeLength = 24;
	float touchdownStripeWidth = 2;
	float touchdownStripeGap = 1.5f;

	float aimingPointStartDistance = 300;
	float aimingPointLength = 45;
	float aimingPointWidth = 10;

	float thresholdMarkingWidth = 1.75f;
	float threaholdMarkingStartDistance = 2.0f;
	float threaholdMarkingLength = 45.0f;

	// Side stripes
	c.generator->addQuad(osg::Vec2f(-c.halfLength, -c.halfWidth), osg::Vec2f(c.halfLength, -c.halfWidth + sideStripeWidth), c.whiteUv, c.whiteUv);
	c.generator->addQuad(osg::Vec2f(-c.halfLength, c.halfWidth - sideStripeWidth), osg::Vec2f(c.halfLength, c.halfWidth + sideStripeWidth), c.whiteUv, c.whiteUv);

	// Runway number
	{
		auto numberAndLetter = splitRunwayNumberAndLetter(c.markingText);
		std::string numberStr = numberAndLetter.first;
		float width = maxCharacterWidth * numberStr.size() + (numberStr.size() - 1) * interLetterGap;
		float y = -width * 0.5f + maxCharacterWidth * 0.5f;
		for (int i = 0; i < numberStr.size(); ++i)
		{
			char digit = numberStr[i];
			auto it = c.atlas->characterBounds.find(digit);
			if (it != c.atlas->characterBounds.end())
			{
				const Box2f& uvBounds = it->second;
				osg::Vec2f size = uvBounds.size();
				float halfCharacterWidth = size.x() / size.y() * textHeight * 0.5f;

				c.generator->addQuad(
					osg::Vec2f(-c.halfLength + markingNumberStartDistance, y - halfCharacterWidth),
					osg::Vec2f(-c.halfLength + markingNumberStartDistance + textHeight, y + halfCharacterWidth), uvBounds.minimum, uvBounds.maximum);
				y += maxCharacterWidth + interLetterGap;
			}
			else
			{
				BOOST_LOG_TRIVIAL(error) << "No font character for runway digit '" << digit << "'";
			}
		}

		auto letterStr = numberAndLetter.second;
		if (!letterStr.empty())
		{
			char letter = std::toupper(letterStr.front());
			auto it = c.atlas->characterBounds.find(letter);
			if (it != c.atlas->characterBounds.end())
			{
				const Box2f& uvBounds = it->second;
				c.generator->addQuad(
					osg::Vec2f(-c.halfLength + markingLetterStartDistance, -maxCharacterWidth * 0.5f),
					osg::Vec2f(-c.halfLength + markingLetterStartDistance + textHeight, maxCharacterWidth * 0.5f), uvBounds.minimum, uvBounds.maximum);
			}
			else
			{
				BOOST_LOG_TRIVIAL(error) << "No font character for runway letter '" << letter << "'";
			}
		}
	}

	// Add threshold markings
	float markingRegionWidth = c.halfWidth - sideStripeWidth;
	float twiceThresholdMarkingWidth = thresholdMarkingWidth * 2.0f;
	int markingCount = markingRegionWidth / twiceThresholdMarkingWidth;
	for (int i = 0; i < markingCount; ++i)
	{
		float start = -c.halfLength + threaholdMarkingStartDistance;
		float end = start + threaholdMarkingLength;

		c.generator->addQuad(
			osg::Vec2f(start, twiceThresholdMarkingWidth * (i + 0.5f)),
			osg::Vec2f(end, twiceThresholdMarkingWidth * (i + 1.0f)), c.whiteUv, c.whiteUv);

		c.generator->addQuad(
			osg::Vec2f(start, -twiceThresholdMarkingWidth * (i + 1.0f)),
			osg::Vec2f(end, -twiceThresholdMarkingWidth * (i + 0.5f)), c.whiteUv, c.whiteUv);
	}

	// Add touchdown markings
	{
		float zoneDistance[] = {
			touchdownStripesStartDistance,
			touchdownStripesStartDistance * 3,
			touchdownStripesStartDistance * 4,
			touchdownStripesStartDistance * 5,
			touchdownStripesStartDistance * 6
		};

		double zoneMarkingCount[] = { 3, 2, 2, 1, 1 };

		for (int j = 0; j < 5; ++j)
		{
			const float startDistance = zoneDistance[j];
			const int markingCount = zoneMarkingCount[j];

			float y = -c.halfWidth + sideStripeWidth + touchdownStripeWidth + (3 - markingCount) * (touchdownStripeWidth + touchdownStripeGap);
			for (int i = 0; i < markingCount; ++i)
			{
				float start = -c.halfLength + startDistance;
				float end = start + touchdownStripeLength;

				c.generator->addQuad(
					osg::Vec2f(start, y),
					osg::Vec2f(end, y + touchdownStripeWidth), c.whiteUv, c.whiteUv);

				c.generator->addQuad(
					osg::Vec2f(start, -y - touchdownStripeWidth),
					osg::Vec2f(end, -y), c.whiteUv, c.whiteUv);

				y += touchdownStripeWidth + touchdownStripeGap;
			}
		}
	}

	// Aiming point markings
	{
		float y = -c.halfWidth + sideStripeWidth + touchdownStripeWidth;
		float start = -c.halfLength + aimingPointStartDistance;
		float end = start + aimingPointLength;

		c.generator->addQuad(
			osg::Vec2f(start, y),
			osg::Vec2f(end, y + aimingPointWidth), c.whiteUv, c.whiteUv);

		c.generator->addQuad(
			osg::Vec2f(start, -y - aimingPointWidth),
			osg::Vec2f(end, -y), c.whiteUv, c.whiteUv);
	}
}

static void createRunwayMarkings(const Runway& runway, const osg::Matrix& transform, MeshBuffers& buffers, const FontAtlas& atlas)
{
	osg::Vec2f displacement = toVec2f(runway.endPoint - runway.startPoint);
	float length = displacement.length();

	FlatShapesGenerator generator(buffers, transform);

	const Box2f& uvBounds = atlas.characterBounds.find('R')->second;

	RunwayEndMarkingsConfig config;
	config.halfWidth = runway.width * 0.5f;
	config.halfLength = displacement.length() * 0.5f;
	config.whiteUv = uvBounds.center();
	config.markingText = runway.startMarking;
	config.generator = &generator;
	config.atlas = &atlas;

	// Create markings at the first end
	createMarkingsForRunwayEnd(config);

	// Create markings at the other end
	osg::Matrix flippedTransform = transform;
	flippedTransform.preMultRotate(osg::Quat(skybolt::math::piF(), osg::Vec3f(0, 0, 1)));
	generator.setTransform(flippedTransform);
	config.markingText = runway.endMarking;
	createMarkingsForRunwayEnd(config);
}

static osg::Geode* createRunways(const Runways& runways, const osg::ref_ptr<osg::Program>& surfaceProgram, const osg::ref_ptr<osg::Program>& textProgram, osg::Texture2D* surfaceTexture, const FontAtlas& atlas)
{
	MeshBuffers surfaceBuffers;
	MeshBuffers markingBuffers;

	for (size_t i = 0; i < runways.size(); ++i)
	{
		const Runway& runway = runways[i];
		osg::Vec3f centre = (runway.startPoint + runway.endPoint) / 2.0f;
		osg::Vec3f displacement = runway.endPoint - runway.startPoint;
		float heading = std::atan2(-displacement.y(), displacement.x());
		float pitch = -std::sin(displacement.z() / displacement.length());

		osg::Matrix transform;
		transform.setTrans(centre);
		transform.setRotate(osg::Quat(pitch, osg::Vec3f(0, 1, 0)) * osg::Quat(heading, osg::Vec3f(0, 0, -1)));

		createRunwaySurface(runway, transform, surfaceBuffers);
		createRunwayMarkings(runway, transform, markingBuffers, atlas);
	}

    osg::Geode* geode = new osg::Geode();

	osg::Geometry* geo = createGeometry(surfaceBuffers);
	{
		osg::StateSet* ss = geo->getOrCreateStateSet();
		ss->setAttribute(surfaceProgram);
		ss->setTextureAttributeAndModes(0, surfaceTexture);
		ss->addUniform(new osg::Uniform("depthOffset", -0.005f));
	}
	geode->addDrawable(geo);

	geo = createGeometry(markingBuffers);
	{
		osg::StateSet* ss = geo->getOrCreateStateSet();
		ss->setAttribute(textProgram);
		ss->setTextureAttributeAndModes(0, atlas.texture);
		ss->addUniform(new osg::Uniform("depthOffset", -0.01f));
	}
	geode->addDrawable(geo);

    return geode;
}


static osg::ref_ptr<osg::StateSet> createStateSet(const RunwaysBatch::Uniforms& uniforms)
{
	osg::ref_ptr<osg::StateSet> ss(new osg::StateSet);
	ss->addUniform(createUniformSampler2d("albedoSampler", 0));
	ss->addUniform(uniforms.modelMatrix);
	ss->setDefine("ENABLE_DEPTH_OFFSET");
	ss->setDefine("ACCURATE_LOG_Z"); // enabled because geometry is sparsely tessellated

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false);
	ss->setAttributeAndModes(depth);

	return ss;
}

RunwaysBatch::RunwaysBatch(const Runways& runways, const osg::ref_ptr<osg::Program>& surfaceProgram, const osg::ref_ptr<osg::Program>& textProgram)
{
	static FontAtlas atlas = createFontAtlas("0123456789LCR");

	osg::Texture2D* surfaceTexture = new osg::Texture2D(readImageWithCorrectOrientation("Environment/Concrete/Asphalt010_2K_Color.jpg"));
	surfaceTexture->setInternalFormat(toSrgbInternalFormat(surfaceTexture->getInternalFormat()));
	surfaceTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	surfaceTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

	osg::Geode* geode = createRunways(runways, surfaceProgram, textProgram, surfaceTexture, atlas);
	mUniforms.modelMatrix = new osg::Uniform("modelMatrix", osg::Matrixf());

	geode->setStateSet(createStateSet(mUniforms));
	mTransform->addChild(geode);
}

void RunwaysBatch::updatePreRender(const RenderContext& context)
{
	mUniforms.modelMatrix->set(mTransform->getWorldMatrices().front());
}

} // namespace vis
} // namespace skybolt
