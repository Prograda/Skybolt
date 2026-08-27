/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "AssetCommand.h"
#include "ComponentCommand.h"
#include "RunCommand.h"

#include <boost/program_options.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

namespace po = boost::program_options;

namespace {

std::string createTopLevelHelpText()
{
	std::ostringstream stream;
	stream << "Usage:\n";
	stream << "  skybolt <command>\n\n";
	stream << "Commands:\n";
	stream << "  run\n";
	stream << "  asset\n";
	stream << "  component\n";
	stream << "  help\n";
	return stream.str();
}

std::vector<std::string> getArgs(int argc, char** argv)
{
	std::vector<std::string> args;
	for (int i = 1; i < argc; ++i)
	{
		args.emplace_back(argv[i]);
	}
	return args;
}

} // namespace

int main(int argc, char** argv)
{
	try
	{
		auto args = getArgs(argc, argv);
		if (args.empty() || args[0] == "help")
		{
			std::cout << createTopLevelHelpText();
			return EXIT_SUCCESS;
		}

		if (args[0] == "run")
		{
			return handleRunCommand(argc - 1, argv + 1);
		}

		if (args[0] == "asset")
		{
			return handleAssetCommand(args, argc - 2, argv + 2);
		}

		if (args[0] == "component")
		{
			return handleComponentCommand(args, argc - 2, argv + 2);
		}

		throw std::runtime_error("Unknown command. Run 'skybolt help' for usage.");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
