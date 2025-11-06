#include "SimSnapshotRegistry.h"
#include "EngineRoot.h"
#include "Scenario.h"

namespace skybolt {

SimSnapshotRegistry::SimSnapshotRegistry(const SimSnapshotRegistryConfig& config) :
	mEntityFactory(config.entityFactory),
	mTypeRegistry(config.typeRegistry),
	mScenario(config.scenario)
{
	assert(mEntityFactory);
	assert(mTypeRegistry);
	assert(mScenario);
}

void SimSnapshotRegistry::loadSnapshot(const Snapshot& snapshot)
{
	EntityPersistenceFlags entityPersistenceFlags = {
		// Non-serializable entities should persist because they won't exist in the serialized state anyhow.
		.persistNonSerializable = true,
		// User-managed entities should persist because their lifetime is managed by the user (usually from the UI) rather than by serialization state loads.
		.persistUserManaged = true
	};

	readScenario(*mTypeRegistry, *mScenario, mEntityFactory, snapshot.state, entityPersistenceFlags);
}

void SimSnapshotRegistry::saveSnapshotAtCurrentTime()
{
	sim::SecondsD simTime = mScenario->timeSource.getTime();
	
	// Remove existing snapshots at given time
	while (true)
	{
		auto it = findSnapshotIteratorAtTime(simTime);
		if (it == mSnapshots.end())	{ break; }
		mSnapshots.erase(it);
	}

	// Add new snapshot
	nlohmann::json state = writeScenario(*mTypeRegistry, *mScenario);
	mSnapshots.push_back({ simTime, {state} });

	// Keep snapshots vector ordered by time
	std::sort(mSnapshots.begin(), mSnapshots.end(), [](const auto& a, const auto& b) {
		return a.first < b.first;
		});
}

const SimSnapshotRegistry::Snapshot* SimSnapshotRegistry::findSnapshotAtTime(sim::SecondsD simTime, sim::SecondsD epsilon) const
{
	auto foundIt = findSnapshotIteratorAtTime(simTime, epsilon);
	return (foundIt != mSnapshots.end()) ? &foundIt->second : nullptr;
}

SimSnapshotRegistry::SnapshotVector::const_iterator SimSnapshotRegistry::findSnapshotIteratorAtTime(sim::SecondsD simTime, sim::SecondsD epsilon) const
{
	auto foundIt = std::lower_bound(mSnapshots.begin(), mSnapshots.end(), simTime,
		[&](const auto& item, sim::SecondsD simTime) {
		return item.first + epsilon < simTime;
		});

	if (foundIt == mSnapshots.end()) { return foundIt; }

	if (std::abs(foundIt->first - simTime) > epsilon) { return mSnapshots.end(); }

	return foundIt;
}

std::unique_ptr<SimSnapshotRegistry> createSnapshotRegistry(const skybolt::EngineRoot& engineRoot)
{
	SimSnapshotRegistryConfig config;
	config.entityFactory = [entityFactory = engineRoot.entityFactory.get()] (const std::string& templateName, const std::string& instanceName) {
		return entityFactory->createEntity(templateName, instanceName);
	};
	config.typeRegistry = engineRoot.typeRegistry.get();
	config.scenario = engineRoot.scenario.get();
	return std::make_unique<SimSnapshotRegistry>(config);
}

inline bool deltaEquals(sim::SecondsD a, sim::SecondsD b, sim::SecondsD epsilon)
{
	return std::abs(a - b) <= epsilon;
}

void useSnapshotsToResetSimulationStateAtTimelineStart(const std::shared_ptr<SimSnapshotRegistry>& snapshotRegistry, Scenario* scenario)
{
	// Connect action to handle saving snapshot when simulation playback is started from the beginning
	scenario->timeSource.stateChanged.connect([snapshotRegistry, scenario] (const TimeSource::State& state) {
		if (scenario->timelineMode.get() != TimelineMode::Live) { return; }
		
		sim::SecondsD startTime = scenario->timeSource.getRange().start;
		bool isAtStart = deltaEquals(scenario->timeSource.getTime(), startTime, 0.001);
		if (isAtStart && state == TimeSource::StatePlaying) // If playback just started from the beginning
		{
			snapshotRegistry->saveSnapshotAtCurrentTime();
		}
	});

	// Connect action to handle loading snapshot when the simulation time is returned to the start
	scenario->timeSource.timeChanged.connect([snapshotRegistry, scenario] (sim::SecondsD simTime) {
		if (scenario->timelineMode.get() != TimelineMode::Live) { return; }
		
		sim::SecondsD startTime = scenario->timeSource.getRange().start;
		bool isAtStart = deltaEquals(simTime, startTime, 0.001);
		if (isAtStart)
		{
			// Load snapshot for the current time if available.
			if (const SimSnapshotRegistry::Snapshot* snapshot = snapshotRegistry->findSnapshotAtTime(startTime); snapshot)
			{
				snapshotRegistry->loadSnapshot(*snapshot);
			}
		}
	});
}

} // amespace skybolt