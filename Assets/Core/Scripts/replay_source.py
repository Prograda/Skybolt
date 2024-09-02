import logging
import math
from dataclasses import dataclass
from types import SimpleNamespace

import pandas as pd
import skybolt as sb
import skybolt_util as sbu

from scipy.interpolate import interp1d

logger = logging.getLogger(__name__)


@dataclass
class EntityHistory:
    entity_template_name: str
    entity_name: str
    time_created: float
    time_destroyed: float
    state: pd.DataFrame


ENTITY_STATE_COLUMN_NAMES = ["x", "y", "z", "roll", "pitch", "yaw"]


class ReplaySource():
    """
    Plays a pre-recorded simulation from a file.
    Supported file formats: CSV.
    """
    def __init__(self, entity):
        self.properties = SimpleNamespace(replay_filename="", initial_time=0.0)
        self.entities = {}

    def __del__(self):
        world = sb.getGlobalEngineRoot().world
        if hasattr(self, "entities"): # Does not exist if __init__ threw exception
            for _, entity in self.entities.items():
                world.removeEntity(entity)

    # Called from C++
    def property_changed(self, name: str, value: any):
        root = sb.getGlobalEngineRoot()
        if name == "replay_filename":
            filename = root.locateFile(value)
            if not filename:
                logger.error("Could not find file: " + value)
                self.entity_histories = {}
            else:
                self.entity_histories = ReplayLoader(filename).entity_histories

            self.set_sim_time(root.scenario.time)

        elif name == "initial_time":
            self.set_sim_time(root.scenario.time)

    # Called from C++.
    def set_sim_time(self, time: float):
        root = sb.getGlobalEngineRoot()

        replay_time = time - self.properties.initial_time
        relevant_entity_histories = {k: v for k, v in self.entity_histories.items() if in_time_range(v, replay_time)}

        # Add or update entities
        for entity_id, history in relevant_entity_histories.items():
            entity = self.entities.get(entity_id)
            if entity is None:
                # Add new entity
                entity = root.entityFactory.createEntity(history.entity_template_name, history.entity_name, sb.Vector3(), sb.Quaternion())
                entity.dynamicsEnabled = False

                metadata = entity.getFirstComponentOfType("ScenarioMetadataComponent")
                metadata.serializable = False
                metadata.deletable = False

                root.world.addEntity(entity)
                self.entities[entity_id] = entity

            # Update entity state
            times = history.state.index.to_numpy()
            values = history.state[ENTITY_STATE_COLUMN_NAMES].values.T

            value = interp1d(times, values, bounds_error=False, fill_value="extrapolate")(replay_time)
            position = sb.Vector3(value[0], value[1], value[2])
            entity.setPosition(position)

            rpy = sb.Vector3(value[3], value[4], value[5])
            lla = sbu.geocentric_to_lla(position)
            entity.setOrientation(sbu.rpy_to_geocentric_quat(rpy, sb.toLatLon(lla)))

        # Remove stale entities
        to_remove = [entity_id for entity_id in self.entities.keys() if entity_id not in relevant_entity_histories]
        for entity_id in to_remove:
            entity = self.entities[entity_id]
            root.world.removeEntity(entity)
            del self.entities[entity_id]


class ReplayLoader:
    def __init__(self, filename: str):
        self.entity_histories = {}

        event_data = pd.read_csv(filename, comment="#", skipinitialspace=True)
        for _, event in event_data.iterrows():
            time = float(event["time"])

            entity_id = int(event["entity_id"])
            if event["event"] == "create_entity":
                self.create_entity(time, entity_id, entity_template_name=event["arg1"], entity_name=event["arg2"])
            elif event["event"] == "destroy_entity":
                self.destroy_entity(time, entity_id)
            elif event["event"] == "update_entity_state":
                self.update_entity_state(time, entity_id, event)

    def create_entity(self, time: float, entity_id: int, entity_template_name: str, entity_name: str):
        state = pd.DataFrame(
            {
                "time": pd.Series(dtype="float"),
                "x": pd.Series(dtype="float"),
                "y": pd.Series(dtype="float"),
                "z": pd.Series(dtype="float"),
                "roll": pd.Series(dtype="float"),
                "pitch": pd.Series(dtype="float"),
                "yaw": pd.Series(dtype="float"),
            }
        )
        state.set_index("time", inplace=True)
        history = EntityHistory(entity_template_name=entity_template_name, entity_name=entity_name, time_created=time, time_destroyed=None, state=state)
        self.entity_histories[entity_id] = history

    def destroy_entity(self, time: float, entity_id: int):
        self.entity_histories[entity_id].time_destroyed = time

    def update_entity_state(self, time: float, entity_id: int, event):
        lla = sb.LatLonAlt(math.radians(float(event["arg1"])), math.radians(float(event["arg2"])), float(event["arg3"]))

        roll = math.radians(float(event["arg4"]))
        pitch = math.radians(float(event["arg5"]))
        yaw = math.radians(float(event["arg6"]))

        xyz = sbu.lla_to_geocentric(lla)

        state = self.entity_histories[entity_id].state

        if len(state.index) > 0:
            prev_row = state.iloc[-1]
            roll = correct_rotation_discontinuity(prev_row["roll"], roll)
            pitch = correct_rotation_discontinuity(prev_row["pitch"], pitch)
            yaw = correct_rotation_discontinuity(prev_row["yaw"], yaw)

        state.loc[time] = [xyz.x, xyz.y, xyz.z, roll, pitch, yaw]


def in_time_range(h: EntityHistory, time: float) -> bool:
    return time >= h.time_created and (h.time_destroyed is None or time < h.time_destroyed)


def correct_rotation_discontinuity(prev_angle: float, new_angle: float) -> float:
    """
    Adjust the new_angle to be within the smallest angular difference from prev_angle.
    Both angles are in radians.
    """
    return prev_angle + sb.calcSmallestAngleFromTo(prev_angle, new_angle)


def skybolt_register():
    root = sb.getGlobalEngineRoot()
    sb.registerComponent(root, ReplaySource)
