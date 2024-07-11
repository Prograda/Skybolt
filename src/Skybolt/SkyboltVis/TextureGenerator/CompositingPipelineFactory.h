/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Group>
#include <osg/Texture>
#include <memory>

namespace skybolt {
namespace vis {

struct CompositingStage
{
	struct Input
	{
		Input(const std::string& samplerName, const osg::ref_ptr<osg::Texture>& texture) : samplerName(samplerName), texture(texture) {}
		std::string samplerName;
		osg::ref_ptr<osg::Texture> texture;
	};

	osg::ref_ptr<osg::Program> program;
	std::vector<Input> inputs;
	std::vector<osg::ref_ptr<osg::Texture>> outputs;
	std::set<osg::ref_ptr<osg::Uniform>> uniforms;
	std::vector<bool> additive; //!< If true, output will be added to previous stage
	bool clear = false; //!< If true, output will be clear to 0,0,0,0 first before rendering
};

class CompositingPipelineFactory
{
public:
	CompositingPipelineFactory();
	~CompositingPipelineFactory();

	osg::ref_ptr<osg::Group> createCompositingPipeline(const std::vector<CompositingStage>& stages);

private:
	std::unique_ptr<class TextureGeneratorCameraFactory> mTextureGeneratorCameraFactory;
};


} // namespace skybolt
} // namespace vis