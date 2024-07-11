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

class Arrows;
class AsyncTileLoader;
class Beams;
class Billboard;
class BillboardForest;
class BuildingsBatch;
struct BuildingTypes;
class Camera;
struct CameraRenderContext;
class Cloud;
struct DetailMappingTechnique;
struct DisplaySettings;
class ElevationProvider;
class EmbeddedWindow;
class LakesBatch;
class Light;
class LlaToNedConverter;
class GpuForest;
class GpuForestTile;
class GpuTextureGenerator;
class JsonTileSourceFactoryRegistry;
class Model;
class ModelFactory;
struct OsgTile;
class OsgTileFactory;
class PagedForest;
class PlanetFeatures;
struct PlanetSubdivisionPredicate;
struct PlanetTileSources;
class QuadTreeTileLoader;
class Ocean;
class Particles;
class Planet;
class PlanetSurface;
class PlanetSky;
class Polyline;
struct RenderContext;
class RenderCameraViewport;
class RenderOperation;
class RenderOperationSequence;
class RenderTarget;
class RenderTexture;
class RoadsBatch;
class RunwaysBatch;
class RootNode;
class Scene;
class ScreenQuad;
class ShaderPrograms;
class ShaderSourceFileChangeMonitor;
class ShadowMapGenerator;
class StandaloneWindow;
class Starfield;
class Terrain;
class TextureCache;
class TextureCompiler;
struct TileImage;
struct TileImages;
class TileImagesLoader;
struct TileProgressCallback;
class TileSource;
struct TileTexture;
class VisMarker;
class VisRoot;
class Viewport;
class VisObject;
class Viewport;
class VolumeClouds;
class Window;

typedef shared_ptr<Arrows> ArrowsPtr;
typedef shared_ptr<AsyncTileLoader> AsyncTileLoaderPtr;
typedef shared_ptr<Beams> BeamsPtr;
typedef shared_ptr<BillboardForest> BillboardForestPtr;
typedef shared_ptr<Billboard> BillboardPtr;
typedef shared_ptr<BuildingsBatch> BuildingsBatchPtr;
typedef shared_ptr<BuildingTypes> BuildingTypesPtr;
typedef shared_ptr<Camera> CameraPtr;
typedef shared_ptr<Cloud> CloudPtr;
typedef shared_ptr<DetailMappingTechnique> DetailMappingTechniquePtr;
typedef shared_ptr<ElevationProvider> ElevationProviderPtr;
typedef shared_ptr<GpuForest> GpuForestPtr;
typedef shared_ptr<GpuForestTile> GpuForestTilePtr;
typedef shared_ptr<JsonTileSourceFactoryRegistry> JsonTileSourceFactoryRegistryPtr;
typedef shared_ptr<LakesBatch> LakesBatchPtr;
typedef shared_ptr<Light> LightPtr;
typedef shared_ptr<LlaToNedConverter> LlaToNedConverterPtr;
typedef shared_ptr<Model> ModelPtr;
typedef shared_ptr<ModelFactory> ModelFactoryPtr;
typedef shared_ptr<Ocean> OceanPtr;
typedef shared_ptr<OsgTile> OsgTilePtr;
typedef shared_ptr<OsgTileFactory> OsgTileFactoryPtr;
typedef shared_ptr<PagedForest> PagedForestPtr;
typedef shared_ptr<Particles> ParticlesPtr;
typedef shared_ptr<Planet> PlanetPtr;
typedef shared_ptr<Polyline> PolylinePtr;
typedef shared_ptr<QuadTreeTileLoader> QuadTreeTileLoaderPtr;
typedef shared_ptr<RenderOperationSequence> RenderOperationSequencePtr;
typedef shared_ptr<RenderTexture> RenderTexturePtr;
typedef shared_ptr<RoadsBatch> RoadsBatchPtr;
typedef shared_ptr<RunwaysBatch> RunwaysBatchPtr;
typedef shared_ptr<RootNode> RootNodePtr;
typedef shared_ptr<Scene> ScenePtr;
typedef shared_ptr<TextureCache> TextureCachePtr;
typedef shared_ptr<Terrain> TerrainPtr;
typedef shared_ptr<TileImages> TileImagesPtr;
typedef shared_ptr<TileImagesLoader> TileImagesLoaderPtr;
typedef shared_ptr<TileProgressCallback> ProgressCallbackPtr;
typedef shared_ptr<TileSource> TileSourcePtr;
typedef shared_ptr<VisMarker> VisMarkerPtr;
typedef shared_ptr<VisObject> VisObjectPtr;
typedef shared_ptr<VisRoot> VisRootPtr;
typedef shared_ptr<VolumeClouds> VolumeCloudsPtr;
typedef shared_ptr<Window> WindowPtr;

typedef size_t InstantMs; //!< Instant of time in milliseconds

} // namespace vis
} // namespace skybolt

namespace px_sched
{
class Scheduler;
class Sync;
}
