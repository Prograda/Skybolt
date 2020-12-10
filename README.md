# Skybolt Engine
Skybolt is a real-time planetary environment rendering engine, designed for flight simulators, aerospace R&D, and geospatial applications. Skybolt is written in C++, based on OpenSceneGraph, and supports CIGI for communicating with host applications. Skybolt also features a Python API for easy integration with science and engineering research tools.

The Skybolt repository includes Sprocket, a GUI application for creating scenarios and visualizing data. Sprocket supports python scripting and node-based graphical programming.

![alt text](https://piraxus.com/wp-content/uploads/2020/06/Mountain1-edited-300x162.jpg "Mointain") ![alt text](https://piraxus.com/wp-content/uploads/2020/06/Seattle2-edit-300x162.jpg "Airport")
![alt text](https://piraxus.com/wp-content/uploads/2020/06/Shuttle3-flipped-300x170.jpg "Shuttle in space") ![alt text](https://piraxus.com/wp-content/uploads/2020/11/ShipHeloShot1-300x169.jpg "Ship on ocean")


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
* Integrates with Sprocket R&D GUI platform, with node-based graphical programming system

## Contact
Skybolt/Sprocket created and maintained by Matthew Reid.
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
auto inputPlatform = std::make_shared<InputPlatformOis>(window->getHandle(), window->getWidth(), window->getHeight()));
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

sb.stepOnceAndRenderUntilDone(engine, window, 0.1)
```

## Projects and Dependencies
This repository contains multiple projects, described below, which can be enabled/disabled in CMake. Each project has a different set of dependencies. You only need to obtain dependencies for projects you wish to build. Header-only dependencies can be obtained from the [SkyboltDependenciesHeaderOnly](https://github.com/Piraxus/SkyboltDependenciesHeaderOnly) repository for convenience.

### Skybolt Engine (Required)
The core engine libraries.
Requires:
* [Boost](www.boost.com)
* [GLM (header only)](https://github.com/g-truc/glm)
* [nlohmann/json (header only)](https://github.com/nlohmann/json)
* [OpenInputSystem](https://github.com/wgois/OIS)
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
* Skybolt Python Bindings
* [Qt](https://www.qt.io)
* [ToolWindowManager](https://github.com/Riateche/toolwindowmanager)

#### NodeGraph Plugin
Sprocket plugin providing a node-based graphical programming interface.
Requires:
* [placeholder/NodeEditor](https://github.com/paceholder/nodeeditor)
* [QCodeEditor](https://github.com/cbtek/EasyCodeCreator/tree/master/common/contrib/QCodeEditor)

#### PythonConsole Plugin
Sprocket plugin providing an interactive python console in the Sprocket GUI.
* [QCodeEditor](https://github.com/cbtek/EasyCodeCreator/tree/master/common/contrib/QCodeEditor)

#### Plot Plugin
Sprocket plugin providing a GUI for plotting graphs.
Requires:
* [Qwt](https://github.com/opencor/qwt)

#### SequenceEditor Plugin
Sprocket plugin providing a GUI for editing animated sequences.
Requires:
* [Qwt](https://github.com/opencor/qwt)

### MapFeaturesConverter Tool
Application for converting Open Street Map data to Skybolt feature tile format.
* [ReadOSM](https://www.gaia-gis.it/fossil/readosm/index)

## Building
Use CMake to configure and generate a build. Optional projects within the repository can be enabled/disabled with CMake BUILD_xxx properties as desired.

## Installing Asset Packages
Skybolt requires runtime assets which are not included in this repository. Assets are split up into separate packages, which can be downloaded from https://piraxus.com/downloads/assetpackages.html

Only the Core package is required to run, but additional packages will add extra assets. To install a package, extract the zip file to a folder called "Assets" in the program working directory.

## Running
### Working Directory
1. Ensure the engine can find shaders (included in this code repository) under either Assets/Shaders or Source/Assets/Shaders relative to the working directory.
2. Ensure the engine can find asset models under Assets/ relative to the working directory.

### Settings
Engine settings are stored in a json file, which may be manually edited with a text editor, or edited in Sprocket with the Tools->Settings dialog.
An example settings file template available at src/SkyboltExamples/ExamplesCommon/ExampleSettings.json.

The settings file can be loaded by example applications with the --settingsFile commandline option. If the option is not specified, a default Settings.json in the Operating System user's home directory will be used. On windows, this is located at C:/Users/<Username>/AppData/Local/Skybolt/Settings.json.

### Using third party Map APIs
By default, the PlanetEarth entity uses mapbox for albedo and elevation data. To use mapbox, you must acquire an API key from https://mapbox.com
If desired, PlanetEarth can be edited to use Bing maps for albedo instead. A bing key can be obtained from https://docs.microsoft.com/en-us/bingmaps/getting-started/bing-maps-dev-center-help/getting-a-bing-maps-key
Keys are stored in the engine json settings file (see above).