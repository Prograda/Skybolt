# Asset Packages
At runtime, Skybolt uses assets such as meshes, textures, and shaders. These assets are organized into packages. Each package is a folder containing a hierarchy of asset files on disk.

Skybolt searches for asset packages in these locations:

1. `<CurrentWorkingDirectory>/Assets`
2. `<CurrentWorkingDirectory>/../Assets`
3. Paths in the `SKYBOLT_ASSETS_PATH` environment variable

## Core Packages
Core packages required for running Skybolt example applications are located in the `Assets` folder of the Skybolt repository.

## Optional Packages
### "NLCDLandCover"
Land cover tiles for USA. Used by Skybolt to place trees on terrain in forest areas. This package can be downloaded [here](https://f000.backblazeb2.com/file/skybolt/NLCDLandCover_1_0_0.zip).

### "Seattle"
Map features (buildings, roads, lakes etc) for the city of Seattle. These features were generated from OpenStreetMap data using Skybolt's `MapFeaturesConverter` tool. This package can be downloaded [here](https://f000.backblazeb2.com/file/skybolt/Seattle_1_1_0.zip).
