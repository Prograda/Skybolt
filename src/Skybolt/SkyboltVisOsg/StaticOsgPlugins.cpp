#include "StaticOsgPlugins.h"

#ifdef OSG_LIBRARY_STATIC
#include <osgDB/Registry>
// include the plugins we need
USE_OSGPLUGIN(bmp)
#ifdef BUILD_WITH_OSG_CURL_PLUGIN
	USE_OSGPLUGIN(curl)
#endif
USE_OSGPLUGIN(dds)
USE_OSGPLUGIN(freetype)
USE_OSGPLUGIN(jpeg)
USE_OSGPLUGIN(png)
USE_OSGPLUGIN(tga)

USE_OSGPLUGIN(osg2)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

// include the platform specific GraphicsWindow implementation
//USE_GRAPHICSWINDOW()
#endif

int ensureStaticOsgPluginsUsed()
{
	return 0;
}