# Skybolt Engine
Skybolt is a real-time planetary environment rendering engine, designed for flight simulators, aerospace R&D, and geospatial applications. Skybolt is written in C++, based on OpenSceneGraph, and supports CIGI for communicating with host applications. Skybolt also features a Python API for easy integration with science and engineering research tools.

The Skybolt repository includes Sprocket, a GUI application providing a sandbox for interactive scenario execution and testing.

![alt text](https://piraxus.com/wp-content/uploads/2020/06/Mountain1-edited-300x162.jpg "Mointain") ![alt text](https://piraxus.com/wp-content/uploads/2020/06/Seattle2-edit-300x162.jpg "Airport")
![alt text](https://piraxus.com/wp-content/uploads/2020/06/Shuttle3-flipped-300x170.jpg "Shuttle in space") ![alt text](https://piraxus.com/wp-content/uploads/2020/11/ShipHeloShot1-300x169.jpg "Ship on ocean")


## Features
* Realistic environment rendering at a range of levels of detail, from ground level through to outer space
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
* Sprocket GUI application, a sandbox for interactive scenario execution and testing

## Contact
Skybolt created and maintained by Matthew Reid.
To submit a bug report, please [raise an issue on the GitHub repository](https://github.com/Piraxus/Skybolt/issues).
For other queries, please use the [contact form](https://piraxus.com/contact) on the [Piraxus website](https://piraxus.com).

## License
This project is licensed under the Mozilla Public License Version 2.0 - see the [License.txt](License.txt) file for details.

## Example Usage (C++)
```cpp
// Create engine
auto params = EngineCommandLineParser::parse(argc, argv);
std::unique_ptr<EngineRoot> root = EngineRootFactory::create(params);

// Create camera
EntityFactory& entityFactory = *root->entityFactory;
World& world = *root->simWorld;
EntityPtr simCamera = entityFactory.createEntity("Camera");
world.addEntity(simCamera);

// Attach camera to window
auto window = std::make_unique<StandaloneWindow>(RectI(0, 0, 1080, 720));
osg::ref_ptr<vis::RenderTarget> viewport = createAndAddViewportToWindowWithEngine(*window, *root);
viewport->setCamera(getVisCamera(*simCamera));

// Create input
auto inputPlatform = std::make_shared<InputPlatformOsg>(window->getViewerPtr());
std::vector<LogicalAxisPtr> axes = CameraInputSystem::createDefaultAxes(*inputPlatform);
root->systemRegistry->push_back(std::make_shared<InputSystem>(inputPlatform, window.get(), axes));
root->systemRegistry->push_back(std::make_shared<CameraInputSystem>(window.get(), simCamera, inputPlatform, axes));

// Create entities
world.addEntity(entityFactory.createEntity("SunBillboard"));
world.addEntity(entityFactory.createEntity("MoonBillboard"));
world.addEntity(entityFactory.createEntity("Stars"));

EntityPtr planet = entityFactory.createEntity("PlanetEarth");
world.addEntity(planet);

// Point camera at planet
auto cameraController = simCamera->getFirstComponentRequired<CameraControllerComponent>()->cameraController;
cameraController->setTarget(planet.get());

// Run loop
runMainLoop(*window, *root, UpdateLoop::neverExit);
```
## Example Usage (Python)
```python
import skybolt as sb

window = sb.StandaloneWindow(sb.RectI(0,0,800,600))
engine = sb.createEngineRootWithDefaults()

camera = engine.entityFactory.createEntity("Camera")
engine.world.addEntity(camera);

sb.attachCameraToWindowWithEngine(camera, window, engine)

engine.world.addEntity(engine.entityFactory.createEntity("SunBillboard"))
engine.world.addEntity(engine.entityFactory.createEntity("MoonBillboard"))
engine.world.addEntity(engine.entityFactory.createEntity("Stars"))

earth = engine.entityFactory.createEntity("PlanetEarth")
engine.world.addEntity(earth);

controller = camera.getFirstComponentOfType("CameraControllerComponent").cameraController
controller.setTarget(earth)
controller.selectController("Globe")

sb.render(engine, window)
```

## Projects and Dependencies
This repository contains multiple projects, described below, which can be enabled/disabled in CMake. Each project has a different set of dependencies. You only need to obtain dependencies for projects you wish to build. Header-only dependencies can be obtained from the [SkyboltDependenciesHeaderOnly](https://github.com/Piraxus/SkyboltDependenciesHeaderOnly) repository for convenience.

### Skybolt Engine (Required)
The core engine libraries.
Requires:
* [Boost](www.boost.com)
* [GLM (header only)](https://github.com/g-truc/glm)
* [nlohmann/json (header only)](https://github.com/nlohmann/json)
* [OpenSceneGraph](https://github.com/openscenegraph/OpenSceneGraph)

### Skybolt Python Bindings
Python bindings for Skybolt.
Requires:
* [Python](https://www.python.org)
* [pybind11 (header only)](https://github.com/pybind/pybind11)

### Bullet Physics Engine Plugin
Skybolt plugin providing a rigid body dynamics component for entities by integrating the Bullet physics engine.
Requires:
* [bullet3](https://github.com/bulletphysics/bullet3)

### CIGI Plugin
Skybolt plugin providing a means for host applications to drive the simulation via the Common Image Generator Interface (CIGI).
Requires:
* [CIGI Class Library](http://cigi.sourceforge.net/product_ccl.php)

### FFT Ocean Plugin
Skybolt plugin simulating ocean waves with FFT.
Requires:
* [muFFT](https://github.com/Themaister/muFFT)
* [Xsimd (header only)](https://github.com/xtensor-stack/xsimd)

### JSBSim Plugin
Skybolt plugin providing an aircraft dynamics component for entities by integrating the JSBSim dynamics engine.
Requires:
* [JSBSim Library](https://github.com/JSBSim-Team/jsbsim)

### Sprocket
GUI application for creating, editing and running simulation scenarios, and performing analysis.
Requires:
* [OpenInputSystem](https://github.com/wgois/OIS)
* [Qt](https://www.qt.io)
* [ToolWindowManager](https://github.com/Riateche/toolwindowmanager)

#### SequenceEditor Plugin (Experimental)
Experimental plugin providing a GUI for editing animated sequences.
Requires:
* [Qwt](https://github.com/opencor/qwt)

### MapFeaturesConverter Tool
Application for converting Open Street Map data to Skybolt feature tile format.
* [ReadOSM](https://www.gaia-gis.it/fossil/readosm/index)

## Building
Use CMake to configure and generate a build. Optional projects within the repository can be enabled/disabled with CMake BUILD_xxx properties as desired.

## Installing Asset Packages
At runtime, Skybolt uses assets such as meshes, textures, and shaders. These assets are organized into packages. Each package is a folder containing a hierarchy of asset files on disk.

Skybolt searches for asset packages in these locations:
1. <CurrentWorkingDirectory>/Assets
2. Paths in the SKYBOLT_ASSETS_PATH environment variable

To run Skybolt, you must ensure the required packages are avilable to Skybolt using either of the above mechanisms.

### Core Package (Required)
Skybolt cannot run without the Core package. It is located under /Assets in this repository.

### SkyboltAssets Packages (Required for Example Applications)
Additional packages for running the example applications are located in the [SkyboltAssets](https://github.com/Piraxus/SkyboltAssets) repository. SkyboltAssets uses [DVC](https://dvc.org) for remote storage and retrieval of large files which are not stored in the git repository itself.

To checkout the SktboltAssets repository:
1. If you do not already have DVC installed, run `pip install dvc[s3]` to install with [pip](https://pypi.org/project/pip)
2. Clone [SkyboltAssets](https://github.com/Piraxus/SkyboltAssets) and checkout desired git branch/tag
3. Run `dvc pull` command in the SkyboltAssets root directory to fetch the remote files

### NLCD Land Cover (Optional)
Land cover tiles for USA. Used by Skybolt to place trees on terrain in forest areas. This package can be downloaded [here](https://f000.backblazeb2.com/file/skybolt/NLCDLandCover_1_0_0.zip).

### Seattle Map Features (Optional)
Map features (buildings, roads, lakes etc) for the city of Seattle. These features were generated from OpenStreetMap data using the MapFeaturesConverter tool. This package can be downloaded [here](https://f000.backblazeb2.com/file/skybolt/Seattle_1_1_0.zip).

## Running
### Settings
Engine settings are stored in a json file, which may be manually edited with a text editor, or edited in Sprocket with the Tools->Settings dialog.
An example settings file template is available in this repository under src/SkyboltExamples/ExamplesCommon/ExampleSettings.json.

The settings file can be loaded by example applications with the --settingsFile commandline option. If the option is not specified, a default Settings.json in the Operating System user's home directory will be used. On windows, this is located at C:/Users/<Username>/AppData/Local/Skybolt/Settings.json.

### Using third party Map APIs
By default, the PlanetEarth entity uses mapbox for albedo and elevation data. To use mapbox, you must acquire an API key from https://mapbox.com
Without an API key, the tiles will not download, the the planet will not render correctly. If desired, PlanetEarth can be edited to use Bing maps for albedo instead. A bing key can be obtained from https://docs.microsoft.com/en-us/bingmaps/getting-started/bing-maps-dev-center-help/getting-a-bing-maps-key
Keys are stored in the engine json settings file (see above).