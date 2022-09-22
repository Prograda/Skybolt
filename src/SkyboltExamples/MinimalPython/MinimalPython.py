import skybolt as sb

window = sb.StandaloneWindow(sb.RectI(0,0,800,600))
engine = sb.createEngineRootWithDefaults()
camera = engine.entityFactory.createEntity("Camera")
engine.world.addEntity(camera);

sb.attachCameraToWindowWithEngine(camera, window, engine)

engine.world.addEntity(engine.entityFactory.createEntity("SunBillboard"))
engine.world.addEntity(engine.entityFactory.createEntity("MoonBillboard"))
engine.world.addEntity(engine.entityFactory.createEntity("Stars"))

earth = engine.entityFactory.createEntity("PlanetEarth")
engine.world.addEntity(earth);

controller = camera.getFirstComponentOfType("CameraControllerComponent").cameraController
controller.setTarget(earth)
controller.selectController("Globe")

sb.render(engine, window)

input("Press enter to exit")
