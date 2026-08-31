import setup_environment
setup_environment.setup_skybolt_environment() # Environment must be setup before importing skybolt

import skybolt as sb

def test_skybolt_api():
    engine = sb.createEngineRoot(loadPlugins=True)

    # Create scenario
    earth = engine.entityFactory.createEntity("PlanetEarth")
    engine.world.addEntity(earth);

    # Read a property
    ocean = earth.getFirstComponentOfType("OceanComponent")
    wave_height_property = sb.getProperty(engine, ocean, "waveHeight")
    wave_height = wave_height_property.value

    # Save it to a string
    json = engine.saveScenarioToJson()

    # Change a property value
    wave_height_property.value = 10

    # Load the scenario back in from the string
    engine.loadScenarioFromJson(json)

    # Test that the property value was restored
    assert (wave_height_property.value == wave_height)

def test_skybolt_vis_render():
    engine = sb.createEngineRoot(loadPlugins=True)

    vis = sb.createVisRoot(engine)
    vis.setLoadTimingPolicy(sb.LoadTimingPolicy.LoadBeforeRender);

    window = sb.StandaloneWindow(sb.RectI(0,0,800,600))
    vis.addWindow(window)

    # Create scenario
    camera = engine.entityFactory.createEntity("Camera")
    engine.world.addEntity(camera);

    engine.world.addEntity(engine.entityFactory.createEntity("SunBillboard"))
    engine.world.addEntity(engine.entityFactory.createEntity("MoonBillboard"))
    engine.world.addEntity(engine.entityFactory.createEntity("Stars"))

    earth = engine.entityFactory.createEntity("PlanetEarth")
    engine.world.addEntity(earth);

    component = camera.getFirstComponentOfType("CameraControllerComponent")
    component.setTargetId(earth.getId())
    component.selectController("Globe")

    # Render image of planet
    sb.attachCameraToWindowWithEngine(camera, window, vis, engine)
    sb.render(engine, vis)

    # Capture screenshot
    image = sb.captureScreenshot(vis)

    # Assert that pixel near edge of image is black (outer space)
    color = image[100][50]
    assert(color[0] == 0)
    assert(color[1] == 0)
    assert(color[2] == 0)

    # Assert that pixel at center is blue (planet ocean)
    color = image[400][300];
    assert(color[0] < 120)
    assert(color[1] < 120)
    assert(color[2] > 120)
