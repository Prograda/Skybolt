#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include "SkyboltEngine/Scenario/ScenarioSerialization.h"
#include <SkyboltReflect/SkyboltReflectFwd.h>
#include <SkyboltSim/Chrono.h>

#include <vector>
#include <nlohmann/json.hpp>

namespace skybolt {

struct SimSnapshotRegistryConfig
{
	EntityFactoryFn entityFactory;
	refl::TypeRegistry* typeRegistry;
	Scenario* scenario;
};

class SimSnapshotRegistry
{
public:
	struct Snapshot
	{
		nlohmann::json state;
	};

	SimSnapshotRegistry(const SimSnapshotRegistryConfig& config);

	void loadSnapshot(const Snapshot& snapshot);
	void saveSnapshotAtCurrentTime();

	//! @returns snapshot within epsilon of time, otherwise null
	const Snapshot* findSnapshotAtTime(sim::SecondsD simTime, sim::SecondsD epsilon = 0.001) const;

private:
	using SnapshotVector = std::vector<std::pair<sim::SecondsD, Snapshot>>;
	SnapshotVector::const_iterator findSnapshotIteratorAtTime(sim::SecondsD simTime, sim::SecondsD epsilon = 0.001) const;

private:
	const EntityFactoryFn mEntityFactory;
	refl::TypeRegistry* mTypeRegistry;
	Scenario* mScenario;

	//! Time-ordered vector of snapshots
	SnapshotVector mSnapshots;
};

std::unique_ptr<SimSnapshotRegistry> createSnapshotRegistry(const EngineRoot& engineRoot);

/*!
	This method enables behavior which resets the simulation state when the sim time is reset to the beginning of the timeline.
	This is needed because in live mode, the simulation state is updated by simulating forward one time-step at a time.
	If there's a time jump, i.e returning sim time to the beginning of the timeline, there's no easy way to derive the
	new sim state given the current state. Therefore, we have to load a previously saved snapshot at the time instead.
*/
void useSnapshotsToResetSimulationStateAtTimelineStart(const std::shared_ptr<SimSnapshotRegistry>& snapshotRegistry, Scenario* scenario);

} // namespace skybolt