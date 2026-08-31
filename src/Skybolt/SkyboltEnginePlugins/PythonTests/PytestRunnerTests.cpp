/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <Python/PythonInterpreter.h>

#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
#pragma pop_macro("slots")

#include <filesystem>
#include <string>

namespace py = pybind11;

#ifndef PYTEST_SCRIPTS_DIR
#define PYTEST_SCRIPTS_DIR "Scripts"
#endif

// The interpreter must outlive all tests, so it lives at file scope.
// Re-initialisation will be skipped if python interpreter is already running.
static skybolt::PythonInterpreter gInterpreter;

//! Runs pytest programmatically inside the already-running interpreter.
//! @returns the pytest exit code.
static int runPytest(const std::string& directory)
{
    py::module_ pytest = py::module_::import("pytest");

    py::list args;
    args.append(directory);

    py::object result = pytest.attr("main")(args);
    return result.cast<int>();
}

TEST_CASE("All pytest scripts in Scripts directory pass", "[python]")
{
    const std::filesystem::path scriptsDir = PYTEST_SCRIPTS_DIR;

    INFO("Pytest scripts directory: " + scriptsDir.string());
    REQUIRE(std::filesystem::exists(scriptsDir));

    const int exitCode = runPytest(scriptsDir.string());

    // pytest exit codes:
    //   0 – all tests passed
    //   1 – some tests failed
    //   2 – interrupted
    //   3 – internal error
    //   4 – bad CLI usage
    //   5 – no tests collected
    INFO("pytest exit code: " << exitCode);
    CHECK(exitCode == 0);
}

int main(int argc, char* argv[])
{
    return Catch::Session().run(argc, argv);
}
