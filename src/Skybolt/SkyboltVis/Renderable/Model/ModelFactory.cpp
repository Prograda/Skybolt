/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "ModelFactory.h"
#include "ModelPreparer.h"
#include "OsgImageHelpers.h"
#include <SkyboltCommon/MapUtility.h>
#include <osgDB/ReadFile>

using namespace skybolt::vis;

// Traverses node hierarchy and sets internal format of textures to an equivalent sRGB format
class StateSetVisitor : public osg::NodeVisitor
{
public:
	StateSetVisitor() :
		NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
	}

	void apply(osg::Drawable& drawable) override
	{
		osg::StateSet* ss = drawable.getStateSet();
		if (ss)
		{
			apply(*ss);
		}

		traverse(drawable);
	}

	void apply(osg::Node& node) override
	{
		osg::StateSet* ss = node.getStateSet();
		if (ss)
		{
			apply(*ss);
		}

		traverse(node);
	}

	virtual void apply(osg::StateSet& stateSet) = 0;
};

// Traverses node hierarchy and sets internal format of textures to an equivalent sRGB format
class TextureSrgbModifier : public StateSetVisitor
{
public:
	void apply(osg::StateSet& stateSet) override
	{
		for (unsigned int i = 0; i < stateSet.getTextureAttributeList().size(); ++i)
		{
			osg::StateAttribute* sa = stateSet.getTextureAttribute(i, osg::StateAttribute::TEXTURE);
			osg::Texture* texture = dynamic_cast<osg::Texture*>(sa);
			if (texture)
			{
				apply(*texture);
			}
		}
	}

	void apply(osg::Texture& texture)
	{
		texture.setInternalFormat(toSrgbInternalFormat(texture.getInternalFormat()));
	}
};

class MaterialShaderAssignmentsModifier : public StateSetVisitor
{
public:
	MaterialShaderAssignmentsModifier(const NamedStateSetModifiers& modifiers) :
		mModifiers(modifiers)
	{
	}

	void apply(osg::StateSet& stateSet) override
	{
		osg::Material* material = dynamic_cast<osg::Material*>(stateSet.getAttribute(osg::StateAttribute::MATERIAL));
		if (material)
		{
			std::string materialName = material->getName();
			auto modifier = skybolt::findOptional(mModifiers, materialName);
			if (modifier)
			{
				(*modifier)(stateSet, *material);
			}
		}
	}

	void apply(osg::Texture& texture)
	{
		texture.setInternalFormat(toSrgbInternalFormat(texture.getInternalFormat()));
	}

private:
	NamedStateSetModifiers mModifiers;
};

ModelFactory::ModelFactory(const ModelFactoryConfig &config) :
	mStateSetModifiers(config.stateSetModifiers),
	mDefaultProgram(config.defaultProgram)
{
	assert(mDefaultProgram);
}

osg::ref_ptr<osg::Node> ModelFactory::createModel(const std::string& filename)
{
	static std::map<std::string, osg::ref_ptr<osg::Node>> modelCache;
	auto it = modelCache.find(filename);
	if (it == modelCache.end())
	{
		osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(filename);
		if (!model)
		{
			throw skybolt::Exception("Could not load OSG model: " + filename);
		}

		ModelPreparer preparer;
		model->accept(preparer);

		{
			TextureSrgbModifier modifier;
			model->accept(modifier);
		}
		{
			MaterialShaderAssignmentsModifier modifier(mStateSetModifiers);
			model->accept(modifier);
		}

		model->getOrCreateStateSet()->setAttribute(mDefaultProgram); // set default program at top level

		modelCache[filename] = model;
		return model;
	}
	else
	{
		return it->second;
	}
}