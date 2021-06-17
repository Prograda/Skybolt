/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ShaderSourceFileChangeMonitor.h"
#include "OsgShaderHelpers.h"
#include "ThirdParty/FileWatch.h"

#include <osgDB/Registry>
#include <filesystem>

namespace skybolt {
namespace vis {

struct WatcherImpl
{
	WatcherImpl(const filewatch::FileWatch<std::string>& watcher) : watcher(watcher) {}
	filewatch::FileWatch<std::string> watcher;
};

static std::string resolveFilename(const std::string& filename)
{
	return osgDB::Registry::instance()->findDataFile(filename, nullptr, osgDB::CASE_SENSITIVE);
}

static std::set<std::filesystem::path> findShaderDirectories(const NamedProgramsMap& programs)
{
	std::set<std::filesystem::path> directories;
	for (const auto& [name, program] : programs)
	{
		for (unsigned int i = 0; i < program->getNumShaders(); ++i)
		{
			osg::Shader* shader = program->getShader(i);
			auto resolvedFilename = resolveFilename(shader->getFileName());
			if (!resolvedFilename.empty())
			{
				auto directory = std::filesystem::path(resolvedFilename).parent_path();
				directories.insert(directory);
			}
		}
	}
	return directories;
}


ShaderSourceFileChangeMonitor::ShaderSourceFileChangeMonitor(const ShaderPrograms& programs) :
	mPrograms(programs)
{
	std::set<std::filesystem::path> directories = findShaderDirectories(programs.getPrograms());

	// Create watcher for each directory
	for (const auto directory : directories)
	{
		try
		{
			mWatchers.push_back(std::make_shared<WatcherImpl>(filewatch::FileWatch<std::string>(directory.string(), [this, directory](const std::string& filename, const filewatch::Event changeType) {
				std::lock_guard<std::mutex> lock(mChangedFilesMutex);
				mChangedFiles.insert(std::filesystem::path(directory / filename).string());
			})));
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error("Exception creating watcher for directory '" + directory.string() + "': " + e.what());
		}
	}
}

ShaderSourceFileChangeMonitor::~ShaderSourceFileChangeMonitor() = default;

void ShaderSourceFileChangeMonitor::update()
{
	std::set<std::string> changedFiles;
	{
		std::lock_guard<std::mutex> lock(mChangedFilesMutex);
		std::swap(mChangedFiles, changedFiles);
	}

	for (const auto& file : changedFiles)
	{
		reloadShadersUsingFile(file);
	}
}

void ShaderSourceFileChangeMonitor::reloadShadersUsingFile(const std::string& filename)
{
	for (const auto& [programName, program] : mPrograms.getPrograms())
	{
		std::vector<osg::ref_ptr<osg::Shader>> shaders(program->getNumShaders());
		for (unsigned int i = 0; i < program->getNumShaders(); ++i)
		{
			shaders[i] = program->getShader(i);
		}

		for (const auto& shader : shaders)
		{
			if (resolveFilename(shader->getFileName()) == filename)
			{
				// Reload
				program->removeShader(shader);
				program->addShader(vis::readShaderFile(shader->getType(), shader->getFileName()));
			}
		}
	}
}

} // namespace vis
} // namespace skybolt
