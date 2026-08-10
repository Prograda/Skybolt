/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <memory>

namespace skybolt {
namespace vis {

using std::shared_ptr;

class AsyncTileLoader;
struct BuildingTypes;
class Image;
class ImageFactory;
class JsonTileSourceFactoryRegistry;
struct PlanetSubdivisionPredicate;
struct PlanetTileSources;
class QuadTreeTileLoader;
struct TileImage;
struct TileImages;
class TileImagesLoader;
struct TileProgressCallback;
class TileSource;

typedef shared_ptr<AsyncTileLoader> AsyncTileLoaderPtr;
typedef shared_ptr<BuildingTypes> BuildingTypesPtr;
typedef shared_ptr<const Image> ConstImagePtr;
typedef shared_ptr<Image> ImagePtr;
typedef shared_ptr<ImageFactory> ImageFactoryPtr;
typedef shared_ptr<JsonTileSourceFactoryRegistry> JsonTileSourceFactoryRegistryPtr;
typedef shared_ptr<QuadTreeTileLoader> QuadTreeTileLoaderPtr;
typedef shared_ptr<TileImages> TileImagesPtr;
typedef shared_ptr<TileImagesLoader> TileImagesLoaderPtr;
typedef shared_ptr<TileProgressCallback> ProgressCallbackPtr;
typedef shared_ptr<TileSource> TileSourcePtr;

} // namespace vis
} // namespace skybolt

namespace px_sched
{
class Scheduler;
class Sync;
}
