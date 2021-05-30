/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisHud.h"

#include <osg/Geode>
#include <osgText/Text>

namespace skybolt {

class BoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
	osg::BoundingBox computeBound(const osg::Drawable & drawable)
	{
		return osg::BoundingBox(osg::Vec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX), osg::Vec3f(FLT_MAX, FLT_MAX, FLT_MAX));
	}
};

class TextPool
{
public:
	typedef osg::ref_ptr<osgText::Text> TextRef;

	TextPool(size_t maxSize = 500) :
		mMaxSize(maxSize)
	{
	}

	TextPool::TextRef checkout(const osgText::String& message)
	{
		TextRef text;

		TextStringMap::iterator i = mUnusedTexts.find(message);
		if (i != mUnusedTexts.end())
		{
			text = i->second;
			mUnusedTexts.erase(i);
		}
		else
		{
			text = new osgText::Text;
			text->setFont("fonts/verdana.ttf");
			text->setText(message);
			text->setUseDisplayList(false);
			text->setUseVertexBufferObjects(true);
			text->setUseVertexArrayObject(true);
			text->setComputeBoundingBoxCallback(osg::ref_ptr<BoundingBoxCallback>(new BoundingBoxCallback));
		}

		if (mUsedTexts.size() < mMaxSize)
			mUsedTexts.push_back(text);

		return text;
	}

	void checkinAll()
	{
		size_t count = mUsedTexts.size();
		for (size_t i = 0; i < count; ++i)
		{
			mUnusedTexts.insert(TextStringMap::value_type(mUsedTexts[i]->getText(), mUsedTexts[i]));
		}
		mUsedTexts.clear();
	}

private:
	typedef std::multimap<osgText::String, TextRef> TextStringMap;
	TextStringMap mUnusedTexts;
	std::vector<TextRef> mUsedTexts;
	size_t mMaxSize;
};

static const char *passThroughVertSource = {
	"#version 330 \n"
	"in vec4 osg_Vertex;\n"
	"in vec4 osg_MultiTexCoord0;\n"
	"out vec4 texCoord;\n"
	"uniform mat4 osg_ModelViewProjectionMatrix;\n"
	"void main(void)\n"
	"{\n"
	"    gl_Position = osg_ModelViewProjectionMatrix  * osg_Vertex;\n"
	"    texCoord = osg_MultiTexCoord0;\n"
	"}\n"
};

static const char *hudFragSource = {
	"#version 330 \n"
	"out vec4 color;\n"
	"void main(void)\n"
	"{\n"
	"    color = vec4 (0.0, 1.0, 0.0, 1.0);\n"
	"}\n"
};

static const char *maskedHudFragSource = {
	"#version 330 \n"
	"in vec4 texCoord;\n"
	"out vec4 color;\n"
	"uniform sampler2D glyphTexture;\n"
	"void main(void)\n"
	"{\n"
	"    float alpha = texture(glyphTexture, texCoord.xy).a;\n"
	"    color = vec4 (0.0, 1.0, 0.0, alpha);\n"
	"}\n"
};

VisHud::VisHud() :
	mPrimitivesGeode(new osg::Geode),
	mTextGeode(new osg::Geode)
{
	osg::ref_ptr<osg::Camera> camera(new osg::Camera);

    camera->setProjectionMatrix(osg::Matrix::ortho2D(-1,1,-1,1));

    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    camera->setClearMask(0);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(false);

	osg::Shader* passThroughShader = new osg::Shader(osg::Shader::VERTEX, passThroughVertSource);
	// Apply shader for unmasked drawables
	{
		osg::ref_ptr<osg::Program> unmaskedProgram = new osg::Program;
		unmaskedProgram->addShader(passThroughShader);
		unmaskedProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, hudFragSource));

		osg::StateSet *ss = mPrimitivesGeode->getOrCreateStateSet();
		ss->setAttributeAndModes(unmaskedProgram, osg::StateAttribute::ON);
		ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	}

	// Apply shader for masked drawables (e.g text)
	{
		osg::ref_ptr<osg::Program> maskedProgram = new osg::Program;
		maskedProgram->addShader(passThroughShader);
		maskedProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, maskedHudFragSource));

		osg::StateSet *ss = mPrimitivesGeode->getOrCreateStateSet();
		ss->setAttributeAndModes(maskedProgram, osg::StateAttribute::ON);
		ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	}

	mTextPool.reset(new TextPool());
	
	camera->addChild(mPrimitivesGeode);
	camera->addChild(mTextGeode);
	addChild(camera);

	clear();
}

VisHud::~VisHud()
{
}

void VisHud::traverse(osg::NodeVisitor& visitor)
{
	if (visitor.getVisitorType() == osg::NodeVisitor::CULL_VISITOR && mDirty)
	{
		mDirty = false;

		mPrimitivesGeode->removeDrawables(0, mPrimitivesGeode->getNumDrawables());

		// Lines
		{
			osg::Geometry* geom = new osg::Geometry;
			geom->setVertexArray(mLineVertices);
			geom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, mLineVertices->size()));
			geom->setUseDisplayList(false);
			geom->setUseVertexBufferObjects(true);
			geom->setUseVertexArrayObject(true);
			geom->setComputeBoundingBoxCallback(osg::ref_ptr<BoundingBoxCallback>(new BoundingBoxCallback));
			mPrimitivesGeode->addDrawable(geom);
		}

		// Triangles
		{
			osg::Geometry* geom = new osg::Geometry;
			geom->setVertexArray(mQuadVertices);
			geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, mQuadVertices->size()));
			geom->setUseDisplayList(false);
			geom->setUseVertexBufferObjects(true);
			geom->setUseVertexArrayObject(true);
			geom->setComputeBoundingBoxCallback(osg::ref_ptr<BoundingBoxCallback>(new BoundingBoxCallback));
			mPrimitivesGeode->addDrawable(geom);
		}
	}
	Group::traverse(visitor);
}

void VisHud::clear()
{
	mLineVertices = new osg::Vec3Array;
	mQuadVertices = new osg::Vec3Array;
	mTextPool->checkinAll();
	mTextGeode->removeDrawables(0, mTextGeode->getNumDrawables());

	setDirty();
}

void VisHud::setDirty()
{
	mDirty = true;
}

float depth = -0.1;

osg::Vec3 toVec3(const glm::vec2 &p0)
{
	return osg::Vec3(p0.x, p0.y, depth);
}

void VisHud::drawLine(const glm::vec2 &p0, const glm::vec2 &p1)
{
	mLineVertices->push_back(toVec3(p0));
	mLineVertices->push_back(toVec3(p1));

	setDirty();
}

void VisHud::drawLineDashed(const glm::vec2 &p0, const glm::vec2 &p1, const DashedLineParams& params)
{
	// TODO
}

void VisHud::drawText(const glm::vec2 &p, const std::string &message, float rotation, float size)
{
    TextPool::TextRef text = mTextPool->checkout(message);
    text->setPosition(toVec3(p));
	text->setCharacterSize(0.03);
	text->setRotation(osg::Quat(rotation, osg::Vec3f(0,0,1)));

    mTextGeode->addDrawable(text);

	setDirty();
}

void VisHud::drawSolidBox(const glm::vec2 &position, float width, float height)
{
	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;
	mQuadVertices->push_back(toVec3(position + glm::vec2(-halfWidth, -halfHeight)));
	mQuadVertices->push_back(toVec3(position + glm::vec2(halfWidth, -halfHeight)));
	mQuadVertices->push_back(toVec3(position + glm::vec2(halfWidth, halfHeight)));
	mQuadVertices->push_back(toVec3(position + glm::vec2(-halfWidth, halfHeight)));

	setDirty();
}

} // namespace skybolt