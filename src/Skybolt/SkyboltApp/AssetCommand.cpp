#include "AssetCommand.h"
#include "EngineRootUtil.h"
#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/EngineCommandLineParser.h>

#include <iostream>

using namespace skybolt;
namespace po = boost::program_options;

int validateAssets(const po::variables_map& vm)
{
	auto engineRoot = createEngineRoot(vm);

	int validatedCount = 0;
	for (const auto& templateName : engineRoot->entityFactory->getTemplateNames())
	{
		auto entity = engineRoot->entityFactory->createEntity(templateName);
		engineRoot->scenario->world.addEntity(entity);
		engineRoot->scenario->world.removeEntity(entity.get());
		++validatedCount;
	}

	std::cout << "Validated " << validatedCount << " entity templates." << std::endl;
	return EXIT_SUCCESS;
}

std::string createAssetHelpText()
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt asset <command>\n\n";
	stream << "Commands:\n";
	stream << "  validate\n";
	stream << "  help\n";
	return stream.str();
}

std::string createAssetValidateHelpText(const po::options_description& options)
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt asset validate [options...]\n\n";
	stream << options;
	return stream.str();
}


po::options_description createAssetValidateOptions()
{
	po::options_description options("Asset validate options");
	EngineCommandLineParser::addOptions(options);
	return options;
}

int handleAssetValidateCommand(int argc, char** argv)
{
	if (argc >= 2 && isHelpToken(argv[1]))
	{
		auto options = createAssetValidateOptions();
		std::cout << createAssetValidateHelpText(options);
		return EXIT_SUCCESS;
	}

	auto options = createAssetValidateOptions();
	po::positional_options_description positional;
	auto vm = parseCommandLine(argc, argv, options, positional);
	if (vm.count("help"))
	{
		std::cout << createAssetValidateHelpText(options);
		return EXIT_SUCCESS;
	}
	return validateAssets(vm);
}

int handleAssetCommand(const std::vector<std::string>& args, int argc, char** argv)
{
	if (args.size() < 2 || isHelpToken(args[1]))
	{
		std::cout << createAssetHelpText();
		return EXIT_SUCCESS;
	}

	if (args[1] == "validate")
	{
		return handleAssetValidateCommand(argc, argv);
	}

	throw std::runtime_error("Unknown asset command. Run 'skybolt help' for usage.");
}