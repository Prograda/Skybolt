#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <memory>
#include <boost/program_options.hpp>
#include <filesystem>
#include <nlohmann/json.hpp>

std::vector<std::filesystem::path> loadDefaultPluginFilepaths();

nlohmann::json readEngineSettingsFromCommandLine(const boost::program_options::variables_map& vm);

std::unique_ptr<skybolt::EngineRoot> createEngineRootWithoutPlugins(const boost::program_options::variables_map& vm);

std::unique_ptr<skybolt::EngineRoot> createEngineRoot(const boost::program_options::variables_map& vm);

boost::program_options::variables_map parseCommandLine(int argc, char** argv, const boost::program_options::options_description& options, const boost::program_options::positional_options_description& positional);

bool isHelpToken(const std::string& arg);