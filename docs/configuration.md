# Configuration
## Engine Settings
Engine settings are stored in a `Settings.json` json file, which may be manually edited with a text editor, or edited in the `SkyboltQtApp` using the `Tools->Settings` dialog.

The settings file can be loaded by Skybolt applications with the `--settingsFile` commandline option. If the option is not specified, a default `Settings.json` in the Operating System user's home directory will be used. On windows, this is located at `C:/Users/%USERNAME%/AppData/Local/Skybolt/Settings.json`.

## Environment Variables
* `SKYBOLT_PLUGINS_PATH` sets plugin search locations. The /plugins folder in the application executable's directory is searched in additional to this path.
* `SKYBOLT_CACHE_DIR` sets the directory where cached terrain tiles are read from and written to. If not set, the default directory is C:/Users/%USERNAME%/AppData/Local/Skybolt/Cache
* `SKYBOLT_ASSETS_PATH` sets the search locations for asset packages. 
* `SKYBOLT_MAX_CORES` sets the maximum number of CPU cores the engine may use. If not set, all cores are used.

## Third Party Map Data
By default, the `PlanetEarth` Skybolt entity uses mapbox for albedo and elevation data. To use mapbox, you must acquire an API key from [Mapbox](https://mapbox.com).
Without an API key, the tiles will not download, the the planet will not render correctly. If desired, `PlanetEarth` can be edited to use Bing maps for albedo instead. A bing key can be obtained from [Microsoft](https://docs.microsoft.com/en-us/bingmaps/getting-started/bing-maps-dev-center-help/getting-a-bing-maps-key).

API keys for third party map sources are stored in the engine settings file `Settings.json`.