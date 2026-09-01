#include "ComponentCommand.h"
#include "EngineRootUtil.h"

#include <SkyboltReflect/Reflection.h>
#include <SkyboltCommon/Json/WriteJsonFile.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/Plugin/PluginHelpers.h>
#include <SkyboltSim/Component.h>

#include <iostream>
#include <map>
#include <regex>
#include <set>

using namespace skybolt;
namespace po = boost::program_options;

namespace {

std::string createComponentHelpText()
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt component <command>\n\n";
	stream << "Commands:\n";
	stream << "  dump\n";
	stream << "  help\n";
	return stream.str();
}

std::string createComponentDumpHelpText(const po::options_description& options)
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt component dump <filename> [options...]\n\n";
	stream << options;
	return stream.str();
}

po::options_description createComponentDumpOptions()
{
	po::options_description options("Component dump options");
	options.add_options()
		("filename", po::value<std::string>(), "output json filename");
	EngineCommandLineParser::addOptions(options);
	return options;
}

std::string toPrettyTypeName(const std::string& name)
{
	static std::vector<std::pair<std::regex, std::string>> mapping = {
		{std::regex("struct "), ""},
		{std::regex("class "), ""},
		{std::regex("skybolt::"), ""},
		{std::regex("std::basic_string<char,std::char_traits<char>,std::allocator<char> >"), "string"},
		{std::regex("glm::vec<3,double,0>"), "Vector3"},
		{std::regex("glm::qua<double,0>"), "Quaternion"},
		{std::regex(R"(,\s*std::allocator<[\s\S]+>\s*>)"), ">"},
		{std::regex(R"(,\s*std::less<[^>]+>\s*)"), ""}
	};

	std::string str = name;
	for (const auto& [pattern, replacement] : mapping)
	{
		str = std::regex_replace(str, pattern, replacement);
	}
	return str;
}

nlohmann::json createPropertyJson(const refl::PropertyPtr& property)
{
	nlohmann::json json;
	json["type"] = property->getType() ? toPrettyTypeName(property->getType()->getName()) : std::string();
	json["readOnly"] = property->isReadOnly();
	return json;
}

nlohmann::json createComponentJson(const refl::TypePtr& type)
{
	nlohmann::json properties = nlohmann::json::object();
	for (const auto& [propertyName, property] : type->getProperties())
	{
		properties[propertyName] = createPropertyJson(property);
	}

	nlohmann::json json;
	json["properties"] = std::move(properties);
	return json;
}

std::map<std::string, refl::TypePtr> getComponentTypesByName(refl::TypeRegistry& typeRegistry)
{
	std::map<std::string, refl::TypePtr> result;
	for (const auto& [typeName, type] : typeRegistry.getTypes())
	{
		if (type->isDerivedFrom<sim::Component>())
		{
			result[typeName] = type;
		}
	}
	return result;
}

int dumpComponents(const po::variables_map& vm)
{
	if (!vm.count("filename"))
	{
		throw std::runtime_error("Missing output filename.");
	}

	const std::string filename = vm["filename"].as<std::string>();
	const auto pluginFilepaths = loadDefaultPluginFilepaths();

	auto baseEngineRoot = createEngineRootWithoutPlugins(vm);
	std::set<std::string> foundComponentNames;
	nlohmann::json coreComponents = nlohmann::json::object();
	for (const auto& [typeName, type] : getComponentTypesByName(*baseEngineRoot->typeRegistry))
	{
		foundComponentNames.insert(typeName);
		coreComponents[typeName] = createComponentJson(type);
	}

	nlohmann::json pluginsJson = nlohmann::json::object();
	pluginsJson["_Core"] = std::move(coreComponents);

	for (size_t i = 0; i < pluginFilepaths.size(); ++i)
	{
		std::string pluginName = pluginFilepaths[i].stem().string();
		if (pluginsJson.contains(pluginName))
		{
			SKYBOLT_LOG(warning) << "Duplicate plugin '" << pluginName << "' found. Skipping duplicates.";
			continue;
		}

		// Load all plugins up to the current index to ensure that dependencies are loaded in the correct order.
		std::vector<std::filesystem::path> pluginFilepathSubset(pluginFilepaths.begin(), pluginFilepaths.begin() + i + 1);

		auto engineRoot = createEngineRootWithoutPlugins(vm);

		const auto pluginFactories = loadPluginFactories<Plugin, PluginConfig>(pluginFilepathSubset);
		engineRoot->loadPlugins(pluginFactories);

		nlohmann::json pluginComponents = nlohmann::json::object();
		for (const auto& [typeName, type] : getComponentTypesByName(*engineRoot->typeRegistry))
		{
			if (!foundComponentNames.contains(typeName))
			{
				foundComponentNames.insert(typeName);
				pluginComponents[typeName] = createComponentJson(type);
			}
		}

		pluginsJson[pluginName] = std::move(pluginComponents);
	}

	nlohmann::json output;
	output["module_components"] = std::move(pluginsJson);

	writeJsonFile(output, filename);
	std::cout << "Wrote component metadata to '" << filename << "'." << std::endl;
	return EXIT_SUCCESS;
}

} // namespace

int handleComponentDumpCommand(int argc, char** argv)
{
	if (argc >= 2 && isHelpToken(argv[1]))
	{
		auto options = createComponentDumpOptions();
		std::cout << createComponentDumpHelpText(options);
		return EXIT_SUCCESS;
	}

	auto options = createComponentDumpOptions();
	po::positional_options_description positional;
	positional.add("filename", 1);

	auto vm = parseCommandLine(argc, argv, options, positional);
	if (vm.count("help"))
	{
		std::cout << createComponentDumpHelpText(options);
		return EXIT_SUCCESS;
	}
	return dumpComponents(vm);
}

int handleComponentCommand(const std::vector<std::string>& args, int argc, char** argv)
{
	if (args.size() < 2 || isHelpToken(args[1]))
	{
		std::cout << createComponentHelpText();
		return EXIT_SUCCESS;
	}

	if (args[1] == "dump")
	{
		return handleComponentDumpCommand(argc, argv);
	}

	throw std::runtime_error("Unknown component command. Run 'skybolt help' for usage.");
}
