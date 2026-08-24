#include "SimSnapshotRegistry.h"
#include "EngineRoot.h"
#include "Scenario.h"

namespace skybolt {

using namespace sim;

SimSnapshotRegistry::SimSnapshotRegistry(const SimSnapshotRegistryConfig& config) :
	mEntityFactory(config.entityFactory),
	mTypeRegistry(config.typeRegistry),
	mScenario(config.scenario),
	mSystemRegistry(config.systemRegistry)
{
	assert(mEntityFactory);
	assert(mTypeRegistry);
	assert(mScenario);
	assert(mSystemRegistry);
}

void SimSnapshotRegistry::loadSnapshot(const Snapshot& snapshot)
{
	readScenario(*mTypeRegistry, *mScenario, mEntityFactory, snapshot.state);
	resetSystemsToInitialState(*mSystemRegistry);
}

void SimSnapshotRegistry::saveSnapshotAtCurrentTime()
{
	sim::SecondsD simTime = mScenario->timeSource->getTime();
	
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

const SimSnapshotRegistry::Snapshot* SimSnapshotRegistry::findEarliestSnapshot() const
{
	if (mSnapshots.empty()) { return nullptr; }
	// Assumes snapshots are sorted by time, which is guaranteed by saveSnapshotAtCurrentTime()
	return &mSnapshots.front().second;
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
	config.systemRegistry = engineRoot.systemRegistry;
	return std::make_unique<SimSnapshotRegistry>(config);
}

inline bool deltaEquals(sim::SecondsD a, sim::SecondsD b, sim::SecondsD epsilon = 0.00001)
{
	return std::abs(a - b) <= epsilon;
}

void useSnapshotsToResetSimulationStateAtTimelineStart(const std::shared_ptr<SimSnapshotRegistry>& snapshotRegistry, Scenario* scenario)
{
	// Connect action to handle saving snapshot when simulation playback is started from the beginning
	scenario->timeSource->timeAboutToChange.connect([snapshotRegistry, scenario] (sim::SecondsD oldSimTime, sim::SecondsD newSimTime) {
		if (scenario->timelineMode.get() != TimelineMode::Live) { return; }
		
		sim::SecondsD startTime = scenario->timeSource->getRange().start;
		bool wasAtStart = deltaEquals(oldSimTime, startTime);
		if (wasAtStart && newSimTime > oldSimTime) // If playback just started from the beginning
		{
			snapshotRegistry->saveSnapshotAtCurrentTime();
		}
	});

	// Connect action to handle loading snapshot when the simulation time is returned to the start
	scenario->timeSource->timeChanged.connect([snapshotRegistry, scenario] (sim::SecondsD simTime) {
		if (scenario->timelineMode.get() != TimelineMode::Live) { return; }
		
		sim::SecondsD startTime = scenario->timeSource->getRange().start;
		bool isAtStart = deltaEquals(simTime, startTime);
		if (isAtStart)
		{
			// Load snapshot for the earliest time.
			if (const SimSnapshotRegistry::Snapshot* snapshot = snapshotRegistry->findEarliestSnapshot(); snapshot)
			{
				snapshotRegistry->loadSnapshot(*snapshot);
			}
		}
	});
}

} // namespace skybolt