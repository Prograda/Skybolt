# Building from Source
Skybolt uses the [CMake](https://cmake.org) meta-build system and the [Conan](https://conan.io/) package manager.

A list of skybolt dependencies can be found in `conanfile.py`.

## 1. Install Conan
Install with [pip](https://pypi.org/project/pip): ```pip3 install conan```.

## 2. Build Skybolt
Use conan to build Skybolt. Under the hood this will execute CMake to generate project files for your IDE and compile the project.
```
conan build %SKYBOLT_SOURCE% --output-folder=%SKYBOLT_BUILD% --lockfile-partial
```
Custom configuration options may be supplied with the `-o` argument, for example:
```
conan build %SKYBOLT_SOURCE% --output-folder=%SKYBOLT_BUILD%  --lockfile-partial -o openscenegraph-mr:shared=True -o enable_python=True -o enable_qt=True -o enable_bullet=True -o enable_cigi=True
```
Please refer to `conanfile.py` for a full list of available configuration options.

## 3. Install Asset Packages
Skybolt needs a minimum set of required asset packages to run. These packages are shipped with the pre-built Skybolt version, but must be installed manually when building from source. See [Asset Packages](asset_packages.md) for details.