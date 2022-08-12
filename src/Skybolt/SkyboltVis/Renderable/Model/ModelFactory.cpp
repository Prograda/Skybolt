/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "ModelFactory.h"
#include "ModelPreparer.h"
#include "OsgImageHelpers.h"
#include "OsgStateSetHelpers.h"
#include <SkyboltCommon/MapUtility.h>

#include <osg/Image>
#include <osgDB/ReadFile>

#include <assert.h>

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

enum class TextureUnitIndexTypes
{
	Albedo,
	Normal
};

static osg::Texture* getTextureOfType(osg::StateSet& stateSet, TextureUnitIndexTypes type)
{
	int typeInt = int(type);
	if (typeInt < stateSet.getTextureAttributeList().size())
	{
		osg::StateAttribute* sa = stateSet.getTextureAttribute(typeInt, osg::StateAttribute::TEXTURE);
		osg::Texture* texture = dynamic_cast<osg::Texture*>(sa);
		if (texture)
		{
			return texture;
		}
	}
	return nullptr;
}

// Traverses node hierarchy and sets internal format of textures to an equivalent sRGB format
class TexturePreparer : public StateSetVisitor
{
public:
	void apply(osg::StateSet& stateSet) override
	{
		osg::Texture* texture = getTextureOfType(stateSet, TextureUnitIndexTypes::Albedo);
		if (texture)
		{
			texture->setInternalFormat(toSrgbInternalFormat(texture->getInternalFormat()));
		}

		texture = getTextureOfType(stateSet, TextureUnitIndexTypes::Normal);
		if (texture)
		{
			stateSet.setDefine("ENABLE_NORMAL_MAP");
			stateSet.addUniform(createUniformSampler2d("normalSampler", 1));
			hasNormalMap = true;
		}
	}

	bool hasNormalMap = false;
};

// Converts from Blinn-Phong 'shininess' (specular exponent) to Beckmann roughness
// From http://simonstechblog.blogspot.com/2011/12/microfacet-brdf.html
static float specularExponentToRoughness(float exponent)
{
	return sqrt(2 / (exponent + 2));
}

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

			// Disney BSDF, blender, Adobe Substance etc all scale the artist given specularity value by 0.08 to
			// put the parameter into a more useful range. We do the same here for compatibility with art tools
			// using the Disney BSDF.
			// See Section 3.2 https://blog.selfshadow.com/publications/s2015-shading-course/burley/s2015_pbs_disney_bsdf_notes.pdf
			// Also see 0.08 used in Blender's implementation: https://github.com/blender/blender/blob/master/intern/cycles/kernel/shaders/node_principled_bsdf.osl
			float disneyBrdfSpecularScaleFactor = 0.08f;
			osg::Vec4 specularity = material->getSpecular(osg::Material::FRONT) * disneyBrdfSpecularScaleFactor;

			const float specularExponent = material->getShininess(osg::Material::FRONT);

			if (specularity.length2() > 0 && specularExponent > 0)
			{
				const float roughness = specularExponentToRoughness(specularExponent);

				stateSet.addUniform(new osg::Uniform("specularity", osg::Vec3f(specularity.r(), specularity.g(), specularity.b())));
				stateSet.addUniform(new osg::Uniform("roughness", roughness));
				stateSet.setDefine("ENABLE_SPECULAR");
				stateSet.setDefine("ENABLE_ENVIRONMENT_MAP");
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

		TexturePreparer modifier;
		model->accept(modifier);

		{
			ModelPreparerConfig config;
			config.generateTangents = modifier.hasNormalMap;
			ModelPreparer preparer(config);
			model->accept(preparer);
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