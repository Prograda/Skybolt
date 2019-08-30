# Skybolt Engine
Skybolt generates realistic real-time images of planetary environments, suitable for use in aerospace simulations. Skybolt is open source, written in C++, based on OpenSceneGraph, and supports CIGI for communicating with host applications.

The Skybolt repository includes Sprocket, a GUI application built on the Skybolt engine, which functions as a platform for aerospace research, data analysis and command/control. Sprocket supports python scripting and node-based graphical programming.

## Features
* Realistic environment rendering at multiple levels of detail, from orbit to planet surface
* Tile streaming from geospatial data sources
* Atmospheric scattering
* Volumetric clouds
* 3D Vegetation based on tree coverage data
* FFT-based ocean wave simulation and rendering
* Extensible entity component framework
* [CIGI](https://en.wikipedia.org/wiki/Common_Image_Generator_Interface) support
* [JSBSim](https://github.com/JSBSim-Team/jsbsim) flight dynamics model integration
* [Bullet](https://github.com/bulletphysics/bullet3) physics integration
* Python API
* Node-based graphical programming system

## Contact
Skybolt/Sprocket created and maintained by Matthew Reid.
To submit a bug report, please [raise an issue on the GitHub repository](https://github.com/Piraxus/Skybolt/issues).
For other queries, please use the [contact form](https://piraxus.com/contact) on the [Piraxus Research website](https://piraxus.com).

## License
This project is licensed under the Mozilla Public License Version 2.0 - see the [License.txt](License.txt) file for details.

## Dependencies
Header only dependencies are available in a separate repository: https://github.com/Piraxus/SkyboltDependenciesHeaderOnly

### Required by Skybolt
* [Boost](www.boost.com)
* [GLM (header only)](https://github.com/g-truc/glm)
* [nlohmann/json (header only)](https://github.com/nlohmann/json)
* [OpenInputSystem](https://github.com/wgois/OIS)
* [OpenSceneGraph](https://github.com/openscenegraph/OpenSceneGraph)

### Required by Python bindings
* [Python](https://www.python.org)
* [pybind11 (header only)](https://github.com/pybind/pybind11)

### Required by Skybolt plugins

#### Bullet Physics Engine Plugin
* [bullet3](https://github.com/bulletphysics/bullet3)

#### CIGI Plugin
* [CIGI Class Library](http://cigi.sourceforge.net/product_ccl.php)

#### FFT Ocean Plugin
* [muFFT](https://github.com/Themaister/muFFT)
* [Xsimd (header only)](https://github.com/xtensor-stack/xsimd)

#### JSBSim Plugin
* [JSBSim Library](https://github.com/JSBSim-Team/jsbsim)

### Required by Sprocket
* [Qt](https://www.qt.io)
* [ToolWindowManager](https://github.com/Riateche/toolwindowmanager)

### Required by Sprocket plugins
#### NodeGraph
* [placeholder/NodeEditor](https://github.com/paceholder/nodeeditor)
* [QCodeEditor](https://github.com/cbtek/EasyCodeCreator/tree/master/common/contrib/QCodeEditor)

#### PythonConsole
* [QCodeEditor](https://github.com/cbtek/EasyCodeCreator/tree/master/common/contrib/QCodeEditor)

#### Plot
* [Qwt](https://github.com/opencor/qwt)

#### SequenceEditor
* [Qwt](https://github.com/opencor/qwt)

### Required by MapFeaturesConverter Tool
* [ReadOSM](https://www.gaia-gis.it/fossil/readosm/index)

## Building
Generate project with CMake.

## Installing Data Modules
Skybolt and Sprocket require runtime data. The runtime data is split up into separate modules, which can be downloaded from https://github.com/Piraxus/Skybolt/releases

Only the Core module is required to run, but additional modules will add extra assets. To install a module, extract the zip file to a folder called "Assets" in the program working directory.

## Running
### Working Directory
1. Ensure the engine can find shaders (included in this code repository) under either Assets/Shaders or Source/Assets/Shaders relative to the working directory.
2. Ensure the engine can find asset models under Assets/ relative to the working directory.

### Using Bing Maps
The PlanetEarth entity uses Bing maps for albedo data. To use Bing Maps, you must acquire an API key from https://docs.microsoft.com/en-us/bingmaps/getting-started/bing-maps-dev-center-help/getting-a-bing-maps-key

After acquiring a key, enter the key in Sprocket under the Tools->Settings dialog.
