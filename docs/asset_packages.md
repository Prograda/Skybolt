# Asset Packages
At runtime, Skybolt uses assets such as meshes, textures, and shaders. These assets are organized into packages. Each package is a folder containing a hierarchy of asset files on disk.

Some packages are **requied** to run Skybolt, while others are optional. The pre-built version of Skybolt ships with the minimum set of required packages. If you compiled Skybolt from source, please ensure Skybolt can find the required packages.

Skybolt searches for asset packages in these locations:

1. `<CurrentWorkingDirectory>/Assets`
2. Paths in the `SKYBOLT_ASSETS_PATH` environment variable

## Required Packages
### "Core"
Core assets including fonts, icons, shaders and python modules. Located under the `Assets` folder in the main repository.

### Packages in the SkyboltAssets repository
These packages required for running the `SkyboltQtApp` and example applications. These are located in the [SkyboltAssets](https://github.com/Prograda/SkyboltAssets) repository. SkyboltAssets uses [DVC](https://dvc.org) for remote storage and retrieval of large files which are not stored in the git repository itself.

To checkout the SktboltAssets repository:

1. If you do not already have DVC installed, run `pip install dvc[s3]` to install with [pip](https://pypi.org/project/pip)
2. Clone [SkyboltAssets](https://github.com/Prograda/SkyboltAssets) and checkout desired git branch/tag
3. Run `dvc pull` command in the SkyboltAssets root directory to fetch the remote files

## Optional Packages
### "NLCDLandCover"
Land cover tiles for USA. Used by Skybolt to place trees on terrain in forest areas. This package can be downloaded [here](https://f000.backblazeb2.com/file/skybolt/NLCDLandCover_1_0_0.zip).

### "Seattle"
Map features (buildings, roads, lakes etc) for the city of Seattle. These features were generated from OpenStreetMap data using Skybolt's `MapFeaturesConverter` tool. This package can be downloaded [here](https://f000.backblazeb2.com/file/skybolt/Seattle_1_1_0.zip).
