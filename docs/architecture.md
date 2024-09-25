# Architecture

## Core
The Skybolt engine is based on a flexible entity-component system, which makes it easy to represent both physical and non-physical objects in the world.

### SkyboltSim
At the heart of the engine, the `SkyboltSim` library manages the simulation loop and handles dynamics and system state updates.

The main aspects of the simulation architecture are:

* **Simulation Loop**: Continuously updates the state of entities based on dynamics, user input, and external data.
* **Systems**: Handle core processes such as dynamics simulation.
* **Entities**: Objects in the simulation. Each entity can represent a physical object (e.g. aircraft, ship) or an non-physical object that performs a function.
* **Components**: Modular building blocks giving functionality to entities. Components may define an entity's position, behavior, appearance, or other characteristics.

### SkyboltVis
The `SkyboltVis` library renders visual represenations of entities. The renderer is built to provide physically plausible imagery from ground level to space. The renderer uses **tile-based streaming** from geospatial sources, allowing efficient real-time loading of terrain and features such as buildings, roads, rivers and lakes.

Environments are rendered use multiple **physically-based lighting** techniques:

* Hard-surface BRDFs to render hard objects like aircraft, terrain and buildings
* Planetary atmospheric scattering model
* Volumetric cloud ray-marcher

Under the hood, `SkyboltVis` uses [OpenSceneGraph](https://github.com/openscenegraph/OpenSceneGraph) to manage scene graph state, issue draw calls, and handle 2D/3D asset file IO.

### SkyboltEngine
The `SkyboltEngine` library ties together `SkyboltSim` and `SkyboltVis`, and provides engine-level functionality like user input, scenario management, and plugin support.

## Plugins
To support a wide variety of use cases, the simulation engine is extensible through **C++** and **Python** plugins.

* **C++ Plugins**: Leverages high-performance code for simulation-critical functionality.
* **Python Plugins**: For rapid prototyping, scripting, and extending functionality, for example adding new entity component types.


## Graphical User Interface (GUI)
The application features a **Qt-based** graphical user interface that allows users to interact with entities in the 3D environment. The `SkyboltQt` library provides funcionality for GUI widgets and bindings between the GUI and the engine.