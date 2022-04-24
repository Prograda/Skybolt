This folder contains a modified copy of https://github.com/conan-io/conan-center-index/tree/master/recipes/openscenegraph

The original package is called `openscenegraph`.
This package is called `openscenegraph-mr`.

Modifications:
* Removed 3.6.5 from config.yml
* Added 3.7.0 to `conandata.yml` to point to [OpenSceneGraph fork by matthew-reid](https://github.com/matthew-reid/OpenSceneGraph) which includes [fix for for missing DDS SRGB texture formats](https://github.com/matthew-reid/OpenSceneGraph/commit/8a83d2a4c1b6c10430f4811a7e69cd4d697db839). These textures are required to support the [Orbiter graphics client](https://github.com/Piraxus/OrbiterSkyboltClient).
* Removed patches since they are already applied on the branch used