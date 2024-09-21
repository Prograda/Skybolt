
# Skybolt Engine
Skybolt is a real-time planetary environment rendering engine, designed for flight simulators and geospatial applications, written in C++ and based on OpenSceneGraph.

The repository also includes SkyboltQt, a Qt-based GUI library providing useful functionality for developing geospatial applications. Additionally, it features a Python API for easy integration with research tools, and a CIGI interface for communication with host applications.

![alt text](https://prograda.com/wp-content/uploads/2020/06/Mountain1-edited-300x162.jpg "Mointain") ![alt text](https://prograda.com/wp-content/uploads/2020/06/Seattle2-edit-300x162.jpg "Airport")
![alt text](https://prograda.com/wp-content/uploads/2020/06/Shuttle3-flipped-300x170.jpg "Shuttle in space") ![alt text](https://prograda.com/wp-content/uploads/2020/11/ShipHeloShot1-300x169.jpg "Ship on ocean")


## Features
* Realistic environment rendering at a range of levels of detail, from ground level through to outer space
* Tile streaming from geospatial data sources
* Atmospheric scattering
* Volumetric clouds
* 3D Vegetation based on tree coverage data
* FFT-based ocean wave simulation and rendering
* Extensible entity component framework
* [CIGI](https://en.wikipedia.org/wiki/Common_Image_Generator_Interface) support
* [Bullet](https://github.com/bulletphysics/bullet3) physics integration
* Python API
* Qt widget library for creating GUIs for geospatial applications

## Contact
Skybolt created and maintained by Matthew Reid.
To submit a bug report, please [raise an issue on the GitHub repository](https://github.com/Prograda/Skybolt/issues).
For other queries, please use our [contact form](https://prograda.com/contact).

## License
This project is licensed under the Mozilla Public License Version 2.0 - see the [License.txt](License.txt) file for details.

## Usage Examples
* [Minimal Example (C++)](src/SkyboltExamples/MinimalApp/MinimalApp.cpp)
* [Minimal Example (Python)](src/SkyboltExamples/MinimalPython/MinimalPython.py)
* [Flight Sim App (C++)](src/SkyboltExamples/FlightSimApp/FlightSimApp.cpp)

## Building
Skybolt uses the [CMake](https://cmake.org) meta-build system, and supports the [Conan](https://conan.io/) package manager. Using Conan usage is optional but strongly encouraged, as it automates dependency acquisition  and build configuration.

A list of skybolt dependencies can be found in [conanfile.py](conanfile.py).

### Building With Conan
#### Install Conan
Currently Skybolt only supports conan version 1.x rather than the newer 2.x version. Installed with [pip](https://pypi.org/project/pip): ```pip3 install conan==1.64.1```.

#### Installing Skybolt's Dependencies
To install dependencies with default Skybolt configuraion:
```
conan install %SKYBOLT_SOURCE% --install-folder=%SKYBOLT_BUILD%
```
To use a custom configuration instead, configuration options may be supplied with the -o argument, for example:
```
conan install %SKYBOLT_SOURCE% --install-folder=%SKYBOLT_BUILD% -o openscenegraph-mr:shared=True -o enable_python=True -o enable_qt=True -o enable_bullet=True -o enable_cigi=True
```
Please refer to [conanfile.py](conanfile.py) for a full list of available configuration options.
#### Building Skybolt
Once dependencies have been installed, Skybolt CMake project can be generated and compiled with:
```
conan build %SKYBOLT_SOURCE% --build-folder=%SKYBOLT_BUILD%
```

## Installing Asset Packages
At runtime, Skybolt uses assets such as meshes, textures, and shaders. These assets are organized into packages. Each package is a folder containing a hierarchy of asset files on disk.

Skybolt searches for asset packages in these locations:
1. <CurrentWorkingDirectory>/Assets
2. Paths in the SKYBOLT_ASSETS_PATH environment variable

To run Skybolt, you must ensure the required packages are avilable to Skybolt using either of the above mechanisms.

### Core Package (Required)
Skybolt cannot run without the Core package. It is located under /Assets in this repository.

### SkyboltAssets Packages (Required for Example Applications)
Additional packages for running the example applications are located in the [SkyboltAssets](https://github.com/Prograda/SkyboltAssets) repository. SkyboltAssets uses [DVC](https://dvc.org) for remote storage and retrieval of large files which are not stored in the git repository itself.

To checkout the SktboltAssets repository:
1. If you do not already have DVC installed, run `pip install dvc[s3]` to install with [pip](https://pypi.org/project/pip)
2. Clone [SkyboltAssets](https://github.com/Prograda/SkyboltAssets) and checkout desired git branch/tag
3. Run `dvc pull` command in the SkyboltAssets root directory to fetch the remote files

### NLCD Land Cover (Optional)
Land cover tiles for USA. Used by Skybolt to place trees on terrain in forest areas. This package can be downloaded [here](https://f000.backblazeb2.com/file/skybolt/NLCDLandCover_1_0_0.zip).

### Seattle Map Features (Optional)
Map features (buildings, roads, lakes etc) for the city of Seattle. These features were generated from OpenStreetMap data using the MapFeaturesConverter tool. This package can be downloaded [here](https://f000.backblazeb2.com/file/skybolt/Seattle_1_1_0.zip).

## Running
### Settings
Engine settings are stored in a json file, which may be manually edited with a text editor, or edited in the SkyboltQtApp with the Tools->Settings dialog.
An example settings file template is available in this repository under src/SkyboltExamples/ExamplesCommon/ExampleSettings.json.

The settings file can be loaded by example applications with the --settingsFile commandline option. If the option is not specified, a default Settings.json in the Operating System user's home directory will be used. On windows, this is located at C:/Users/%USERNAME%/AppData/Local/Skybolt/Settings.json.

### Environment Variables
* SKYBOLT_PLUGINS_PATH sets plugin search locations. The /plugins folder in the application executable's directory is searched in additional to this path.
* SKYBOLT_CACHE_DIR sets the directory where cached terrain tiles are read from and written to. If not set, the default directory is C:/Users/%USERNAME%/AppData/Local/Skybolt/Cache
* SKYBOLT_ASSETS_PATH sets the search locations for asset packages. 
* SKYBOLT_MAX_CORES sets the maximum number of CPU cores the engine may use. If not set, all cores are used.

### Using third party Map APIs
By default, the PlanetEarth entity uses mapbox for albedo and elevation data. To use mapbox, you must acquire an API key from https://mapbox.com
Without an API key, the tiles will not download, the the planet will not render correctly. If desired, PlanetEarth can be edited to use Bing maps for albedo instead. A bing key can be obtained from https://docs.microsoft.com/en-us/bingmaps/getting-started/bing-maps-dev-center-help/getting-a-bing-maps-key
Keys are stored in the engine json settings file (see above).