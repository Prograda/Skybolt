/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisHud.h"

#include <osg/Geode>
#include <osgText/Text>

namespace skybolt {

class TextPool
{
public:
	typedef osg::ref_ptr<osgText::Text> TextRef;

	TextPool(osg::ref_ptr<osg::Program> program, size_t maxSize = 500) :
		mProgram(program),
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
			text->getOrCreateStateSet()->setAttributeAndModes(mProgram, osg::StateAttribute::ON);
			text->setUseDisplayList(false);
			text->setUseVertexBufferObjects(true);
			text->setUseVertexArrayObject(true);
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
	osg::ref_ptr<osg::Program> mProgram;
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
	"void main(void)\n"
	"{\n"
	"    gl_FragColor = vec4 (0.0, 1.0, 0.0, 1.0);\n"
	"}\n"
};

static const char *maskedHudFragSource = {
	"#version 330 \n"
	"in vec4 texCoord;\n"
	"uniform sampler2D glyphTexture;\n"
	"void main(void)\n"
	"{\n"
	"    float alpha = texture(glyphTexture, texCoord.xy).a;\n"
	"    gl_FragColor = vec4 (0.0, 1.0, 0.0, alpha);\n"
	"}\n"
};

VisHud::VisHud(float aspectRatio) :
	mCamera(new osg::Camera),
	mGeode(new osg::Geode)
{
    mCamera->setProjectionMatrix(osg::Matrix::ortho2D(-aspectRatio,aspectRatio,-1,1));

    mCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    mCamera->setViewMatrix(osg::Matrix::identity());

    mCamera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    mCamera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    mCamera->setAllowEventFocus(false);

	osg::Shader* passThroughShader = new osg::Shader(osg::Shader::VERTEX, passThroughVertSource);
	// Apply shader for unmasked drawables
	{
		osg::ref_ptr<osg::Program> unmaskedProgram = new osg::Program;
		unmaskedProgram->addShader(passThroughShader);
		unmaskedProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, hudFragSource));

		osg::StateSet *ss = mCamera->getOrCreateStateSet();
		ss->setAttributeAndModes(unmaskedProgram, osg::StateAttribute::ON);
	}

	// Apply shader for masked drawables (e.g text)
	osg::ref_ptr<osg::Program> maskedProgram = new osg::Program;
	maskedProgram->addShader(passThroughShader);
	maskedProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, maskedHudFragSource));

	mTextPool.reset(new TextPool(maskedProgram));

	nextFrame();
}

VisHud::~VisHud()
{
}

void VisHud::updatePreRender(const vis::RenderContext& context)
{
	// Lines
	{
		osg::Geometry* geom = new osg::Geometry;
		geom->setVertexArray(mLineVertices);
		geom->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0, mLineVertices->size()));
		geom->setUseDisplayList(false); 
        geom->setUseVertexBufferObjects(true); 
		geom->setUseVertexArrayObject(true);
		mGeode->addDrawable(geom);
	}

	// Triangles
	{
		osg::Geometry* geom = new osg::Geometry;
		geom->setVertexArray(mQuadVertices);
		geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0, mQuadVertices->size()));
		mGeode->addDrawable(geom);
	}

	nextFrame();
}

void VisHud::nextFrame()
{
	mCamera->removeChildren(0, mCamera->getNumChildren());
	mCamera->addChild(mGeode);

	mLineVertices = new osg::Vec3Array;
	mQuadVertices = new osg::Vec3Array;
	mGeode = new osg::Geode;

	mTextPool->checkinAll();
}

osg::Node* VisHud::_getNode() const {return mCamera;}

float depth = -0.1;

osg::Vec3 toVec3(const glm::vec2 &p0)
{
	return osg::Vec3(p0.x, p0.y, depth);
}

void VisHud::drawLine(const glm::vec2 &p0, const glm::vec2 &p1)
{
	mLineVertices->push_back(toVec3(p0));
	mLineVertices->push_back(toVec3(p1));
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

    mGeode->addDrawable(text);
}

void VisHud::drawSolidBox(const glm::vec2 &position, float height, float width)
{
	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;
	mQuadVertices->push_back(toVec3(position + glm::vec2(-halfWidth, -halfHeight)));
	mQuadVertices->push_back(toVec3(position + glm::vec2(halfWidth, -halfHeight)));
	mQuadVertices->push_back(toVec3(position + glm::vec2(halfWidth, halfHeight)));
	mQuadVertices->push_back(toVec3(position + glm::vec2(-halfWidth, halfHeight)));
}

} // namespace skybolt