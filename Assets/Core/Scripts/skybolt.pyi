from __future__ import annotations
import numpy
import typing
__all__ = ['Box3d', 'CameraComponent', 'CameraControllerComponent', 'CameraControllerSelector', 'CameraState', 'Component', 'EngineRoot', 'Entity', 'EntityFactory', 'EntityId', 'EntityTargeter', 'Frustum', 'GeocentricOrientation', 'GeocentricPosition', 'LatLon', 'LatLonAlt', 'LatLonAltPosition', 'LoadAcrossMultipleFrames', 'LoadBeforeRender', 'LoadTimingPolicy', 'LtpNedOrientation', 'MainRotorComponent', 'OffscreenWindow', 'Orientation', 'Position', 'Quaternion', 'RectI', 'Scenario', 'ScenarioMetadataComponent', 'StandaloneWindow', 'TemplateNameComponent', 'Vector3', 'VectorString', 'VisRoot', 'Window', 'World', 'attachCameraToWindowWithEngine', 'calcSmallestAngleFromTo', 'captureScreenshot', 'createEngineRootWithDefaults', 'cross', 'dot', 'getGlobalEngineRoot', 'moveDistanceAndBearing', 'normalize', 'quaternionFromEuler', 'registerComponent', 'render', 'setGlobalEngineRoot', 'setWaveHeight', 'stepSim', 'toGeocentricOrientation', 'toGeocentricPosition', 'toLatLon', 'toLatLonAlt', 'transformToScreenSpace']
class Box3d:
    maximum: Vector3
    minimum: Vector3
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: Vector3, arg1: Vector3) -> None:
        ...
    def center(self) -> Vector3:
        ...
    def merge(self, arg0: Vector3) -> None:
        ...
    def size(self) -> Vector3:
        ...
class CameraComponent(Component):
    state: CameraState
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class CameraControllerComponent(Component, CameraControllerSelector):
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class CameraControllerSelector:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def getSelectedControllerName(self) -> str:
        ...
    def selectController(self, arg0: str) -> None:
        ...
    def setTargetId(self, arg0: EntityId) -> None:
        ...
class CameraState:
    farClipDistance: float
    fovY: float
    nearClipDistance: float
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self) -> None:
        ...
class Component:
    """
    Base class for components which can be attached to an `Entity`
    """
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self) -> None:
        ...
    def setSimTime(self, arg0: float) -> None:
        ...
class EngineRoot:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def locateFile(self, arg0: str) -> str:
        ...
    @property
    def entityFactory(self) -> EntityFactory:
        ...
    @property
    def scenario(self) -> Scenario:
        ...
    @property
    def world(self) -> World:
        ...
class Entity:
    dynamicsEnabled: bool
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def addComponent(self, arg0: Component) -> None:
        ...
    def getComponents(self) -> list[Component]:
        ...
    def getComponentsOfType(self, arg0: str) -> list[Component]:
        ...
    def getFirstComponentOfType(self, arg0: str) -> Component:
        ...
    def getId(self) -> EntityId:
        ...
    def getName(self) -> str:
        ...
    def getOrientation(self) -> Quaternion:
        ...
    def getPosition(self) -> Vector3:
        ...
    def setOrientation(self, arg0: Quaternion) -> None:
        ...
    def setPosition(self, arg0: Vector3) -> None:
        ...
class EntityFactory:
    """
    Class responsible for creating `Entity` instances based on a template name
    """
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def createEntity(self, templateName: str, name: str = '', position: Vector3 = ..., orientation: Quaternion = ..., id: EntityId = ...) -> Entity:
        ...
class EntityId:
    applicationId: int
    entityId: int
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class EntityTargeter:
    """
    Interface for a class which references a target `Entity` by `EntityId`
    """
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def getTargetId(self) -> EntityId:
        ...
    def getTargetName(self) -> str:
        ...
    def setTargetId(self, arg0: EntityId) -> None:
        ...
    def setTargetName(self, arg0: str) -> None:
        ...
class Frustum:
    fieldOfViewHorizontal: float
    fieldOfViewVertical: float
    orientation: Quaternion
    origin: Vector3
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self) -> None:
        ...
class GeocentricOrientation(Orientation):
    orientation: Quaternion
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self, arg0: Quaternion) -> None:
        ...
class GeocentricPosition(Position):
    position: Vector3
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self, arg0: Vector3) -> None:
        ...
class LatLon:
    """
    Represents a 2D location on a planet location as a latitude and longitude in radians
    """
    lat: float
    lon: float
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float, arg1: float) -> None:
        ...
class LatLonAlt:
    """
    Represents a 3D location on a planet location as a latitude and longitude in radians, and altitude in meters
    """
    alt: float
    lat: float
    lon: float
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float, arg1: float, arg2: float) -> None:
        ...
class LatLonAltPosition(Position):
    position: LatLonAlt
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self, arg0: LatLonAlt) -> None:
        ...
class LoadTimingPolicy:
    """
    Enum specifying policy for timing of loading data into the renderer
    
    Members:
    
      LoadAcrossMultipleFrames
    
      LoadBeforeRender
    """
    LoadAcrossMultipleFrames: typing.ClassVar[LoadTimingPolicy]  # value = <LoadTimingPolicy.LoadAcrossMultipleFrames: 0>
    LoadBeforeRender: typing.ClassVar[LoadTimingPolicy]  # value = <LoadTimingPolicy.LoadBeforeRender: 1>
    __members__: typing.ClassVar[dict[str, LoadTimingPolicy]]  # value = {'LoadAcrossMultipleFrames': <LoadTimingPolicy.LoadAcrossMultipleFrames: 0>, 'LoadBeforeRender': <LoadTimingPolicy.LoadBeforeRender: 1>}
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class LtpNedOrientation(Orientation):
    orientation: Quaternion
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self, arg0: Quaternion) -> None:
        ...
class MainRotorComponent(Component):
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def getPitchAngle(self) -> float:
        ...
    def getRotationAngle(self) -> float:
        ...
    def getTppOrientationRelBody(self) -> Quaternion:
        ...
    def setNormalizedRpm(self, arg0: float) -> None:
        ...
class OffscreenWindow(Window):
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self, arg0: int, arg1: int) -> None:
        ...
class Orientation:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class Position:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class Quaternion:
    """
    Represents an orientation as a quarternion of [x, y, z, w] components
    """
    w: float
    x: float
    y: float
    z: float
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float, arg1: float, arg2: float, arg3: float) -> None:
        ...
    def __mul__(self, arg0: Vector3) -> Vector3:
        ...
class RectI:
    """
    A rectangle defined by [x, y, width, height]
    """
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self, arg0: int, arg1: int, arg2: int, arg3: int) -> None:
        ...
class Scenario:
    startJulianDate: float
    time: float
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @property
    def currentJulianDate(self) -> float:
        ...
class ScenarioMetadataComponent(Component):
    deletable: bool
    directory: VectorString
    serializable: bool
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class StandaloneWindow(Window):
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self, arg0: RectI) -> None:
        ...
class TemplateNameComponent(Component):
    """
    A component storing the name of the template which an `Entity` instantiates
    """
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @property
    def name(self) -> str:
        ...
class Vector3:
    """
    A 3D vector of [x, y, z] components
    """
    x: float
    y: float
    z: float
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: Vector3) -> Vector3:
        ...
    def __iadd__(self, arg0: Vector3) -> Vector3:
        ...
    def __imul__(self, arg0: float) -> Vector3:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float, arg1: float, arg2: float) -> None:
        ...
    def __isub__(self, arg0: Vector3) -> Vector3:
        ...
    def __itruediv__(self, arg0: float) -> Vector3:
        ...
    @typing.overload
    def __mul__(self, arg0: Vector3) -> Vector3:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> Vector3:
        ...
    def __rmul__(self, arg0: float) -> Vector3:
        ...
    def __sub__(self, arg0: Vector3) -> Vector3:
        ...
    @typing.overload
    def __truediv__(self, arg0: Vector3) -> Vector3:
        ...
    @typing.overload
    def __truediv__(self, arg0: float) -> Vector3:
        ...
class VectorString:
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    def __contains__(self, x: str) -> bool:
        """
        Return true the container contains ``x``
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    def __eq__(self, arg0: VectorString) -> bool:
        ...
    @typing.overload
    def __getitem__(self, s: slice) -> VectorString:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> str:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: VectorString) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: typing.Iterable) -> None:
        ...
    def __iter__(self) -> typing.Iterator[str]:
        ...
    def __len__(self) -> int:
        ...
    def __ne__(self, arg0: VectorString) -> bool:
        ...
    def __repr__(self) -> str:
        """
        Return the canonical string representation of this list.
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: str) -> None:
        ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: VectorString) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: str) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    def count(self, x: str) -> int:
        """
        Return the number of times ``x`` appears in the list
        """
    @typing.overload
    def extend(self, L: VectorString) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: typing.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: int, x: str) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> str:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: int) -> str:
        """
        Remove and return the item at index ``i``
        """
    def remove(self, x: str) -> None:
        """
        Remove the first item from the list whose value is x. It is an error if there is no such item.
        """
class VisRoot:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __init__(self) -> None:
        ...
    def addWindow(self, arg0: Window) -> None:
        ...
    def removeWindow(self, arg0: Window) -> None:
        ...
    def setLoadTimingPolicy(self, arg0: LoadTimingPolicy) -> None:
        ...
class Window:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class World:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def addEntity(self, arg0: Entity) -> None:
        ...
    def findObjectByName(self, arg0: str) -> Entity:
        ...
    def getEntities(self) -> list[Entity]:
        ...
    def removeAllEntities(self) -> None:
        ...
    def removeEntity(self, arg0: Entity) -> None:
        ...
def attachCameraToWindowWithEngine(arg0: Entity, arg1: Window, arg2: EngineRoot) -> bool:
    ...
def calcSmallestAngleFromTo(arg0: float, arg1: float) -> float:
    ...
@typing.overload
def captureScreenshot(arg0: VisRoot) -> numpy.ndarray[numpy.uint8]:
    ...
@typing.overload
def captureScreenshot(arg0: VisRoot, arg1: str) -> None:
    ...
def createEngineRootWithDefaults() -> EngineRoot:
    """
    Create an EngineRoot with default values
    """
def cross(arg0: Vector3, arg1: Vector3) -> Vector3:
    ...
def dot(arg0: Vector3, arg1: Vector3) -> float:
    ...
def getGlobalEngineRoot() -> EngineRoot:
    """
    Get global EngineRoot
    """
def moveDistanceAndBearing(arg0: LatLon, arg1: float, arg2: float) -> LatLon:
    ...
def normalize(arg0: Vector3) -> Vector3:
    ...
def quaternionFromEuler(arg0: Vector3) -> Quaternion:
    ...
def registerComponent(arg0: EngineRoot, arg1: typing.Any) -> None:
    ...
def render(engineRoot: EngineRoot, window: VisRoot) -> bool:
    ...
def setGlobalEngineRoot(arg0: EngineRoot) -> None:
    """
    Set global EngineRoot
    """
def setWaveHeight(arg0: Entity, arg1: float) -> None:
    ...
def stepSim(arg0: EngineRoot, arg1: float) -> None:
    ...
def toGeocentricOrientation(arg0: Orientation, arg1: LatLon) -> GeocentricOrientation:
    ...
def toGeocentricPosition(arg0: Position) -> GeocentricPosition:
    ...
def toLatLon(arg0: LatLonAlt) -> LatLon:
    ...
@typing.overload
def toLatLonAlt(arg0: Position) -> LatLonAltPosition:
    ...
@typing.overload
def toLatLonAlt(arg0: LatLon, arg1: float) -> LatLonAlt:
    ...
def transformToScreenSpace(arg0: Frustum, arg1: Vector3) -> Vector3:
    ...
LoadAcrossMultipleFrames: LoadTimingPolicy  # value = <LoadTimingPolicy.LoadAcrossMultipleFrames: 0>
LoadBeforeRender: LoadTimingPolicy  # value = <LoadTimingPolicy.LoadBeforeRender: 1>
