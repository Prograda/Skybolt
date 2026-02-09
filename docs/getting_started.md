# Getting Started

## Obtaining Skybolt

Skybolt can be obtained by either of these methods:

* [Download a pre-built version](https://github.com/prograda/skybolt/releases) on supported operating systems
* [Build from source](building_from_source.md)

## Scenario Viewer App
Skybolt comes with a Qt-based 3D scenario viewer application. Example scenarios that demonstrate use cases of the application can be found in `/Scenarios`.

## Creating Applications with Skybolt
The `src/SkyboltExamples` directory contains example applications demonstrating how to build your own projects using Skybolt libraries.

## Using Python API
Refer to `src/SkyboltExamples/MinimalPython/MinimalPython.py` for example usage of the python API.
Refer to [Python API documentation](python_api/index.md) for complete reference.

## Writing Plugins
Skybolt supports Python and C++ plugins as follows:

### Python
Skybolt allows users to implement new Entity Component types in Python. These components are initialized when the entity is added to the scene, updated each time step, and destroyed when the entity is removed.

The `ReplaySource` is an example of a component implemented in Python. This component is responsible for replaying simulation data from a CSV file. See `Assets/Core/Scripts/replay_source.py`.

At runtime, Skybolt searches for python plugin modules inside asset packages under `<AssetPackage>/Scripts`.

### C++
Skybolt allows develpers to add new engine functionality by writing C++ plugins. Please refer to `src/Skybolt/SkyboltEnginePlugins/` for examples.

At runtime, Skybolt searches for plugins under the `bin/plugins` folder.