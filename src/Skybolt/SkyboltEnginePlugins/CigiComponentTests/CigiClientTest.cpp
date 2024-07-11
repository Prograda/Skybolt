/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <CigiComponent/CigiClient.h>
#include <CigiComponent/UdpCommunicator.h>

#include <SkyboltCommon/Eventually.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/NumericComparison.h>

#include <cigicl/CigiHostSession.h>
#include <cigicl/CigiEntityCtrlV3_3.h>
#include <cigicl/CigiIGCtrlV3.h>
#include <cigicl/CigiViewCtrlV3.h>
#include <cigicl/CigiViewDefV3.h>

#include <chrono>

using namespace skybolt;

class DummyCamera : public CigiCamera
{
public:
	void setParent(const CigiEntityPtr& parent) override
	{
		this->parent = parent;
	}

	void setPositionOffset(const sim::Vector3& position) override
	{
		positionOffset = position;
	}

	void setOrientationOffset(const sim::Vector3& rpy) override
	{
		orientationOffset = rpy;
	}

	void setHorizontalFieldOfView(float fov) override
	{
		horizontalFov = fov;
	}

	void setVerticalFieldOfView(float fov) override
	{
		verticalFov = fov;
	}

	CigiEntityPtr parent;
	sim::Vector3 positionOffset;
	sim::Vector3 orientationOffset;
	float horizontalFov;
	float verticalFov;
};

class DummyEntity : public CigiEntity
{
public:
	DummyEntity(int type) :
		type(type)
	{}

	void setPosition(const sim::LatLonAlt& position) override
	{
		this->position = position;
	}

	void setOrientation(const sim::Vector3& ypr) override
	{
		this->orientation = ypr;
	}

	void setVisible(bool visible)
	{
		this->visible = visible;
	}

	sim::LatLonAlt position;
	sim::Vector3 orientation;
	bool visible = true;
	int type;
};

class DummyWorld : public CigiWorld
{
public:
	CigiEntityPtr createEntity(int typeId) override
	{
		auto entity = std::make_shared<DummyEntity>(typeId);
		entities.insert(entity);
		return entity;
	}

	void destroyEntity(const CigiEntityPtr& entity) override
	{
		entities.erase(entity);
	}

	CigiCameraPtr createCamera() override
	{
		auto camera = std::make_shared<DummyCamera>();
		cameras.insert(camera);
		return camera;
	}

	void destroyCamera(const CigiCameraPtr& camera) override
	{
		cameras.erase(camera);
	}

	std::set<CigiEntityPtr> entities;
	std::set<CigiCameraPtr> cameras;
};

class CigiHost
{
public:
	CigiHost(std::unique_ptr<UdpCommunicator> communicator, int cigiMajorVersion, int cigiMinorVersion) :
		mCommunicator(std::move(communicator))
	{
		mSession.SetCigiVersion(CigiVersionID(cigiMajorVersion, cigiMinorVersion));
		mSession.SetSynchronous(true);
	}

	void send(const std::function<void(CigiOutgoingMsg&)>& messageWriteFunction)
	{
		CigiOutgoingMsg& outgoingMessage = mSession.GetOutgoingMsgMgr();
		outgoingMessage.BeginMsg();
		messageWriteFunction(outgoingMessage);

		Cigi_uint8* message = 0;
		int length = 0;
		outgoingMessage.PackageMsg(&message, length);

		mCommunicator->send(*message, length);

		outgoingMessage.FreeMsg();
	}

private:
	std::unique_ptr<UdpCommunicator> mCommunicator;
	CigiHostSession mSession;
};

static std::unique_ptr<UdpCommunicator> CreateUdpCommunicator()
{
	UdpCommunicatorConfig config;
	config.localAddress = "localhost";
	config.localPort = 8001;
	config.remoteAddress = "localhost";
	config.remotePort = 8002;

	return std::make_unique<UdpCommunicator>(config);
}

static std::unique_ptr<CigiHost> CreateCigiHost(int cigiMajorVersion, int cigiMinorVersion)
{
	auto connection = CreateUdpCommunicator();
	return std::make_unique<CigiHost>(std::move(connection), cigiMajorVersion, cigiMinorVersion);
}

static std::unique_ptr<CigiClient> CreateCigiClient(int cigiMajorVersion, const std::shared_ptr<DummyWorld>& world)
{
	CigiClientConfig config;
	config.cigiMajorVersion = cigiMajorVersion;
	config.host = "localhost";
	config.hostPort = 8001;
	config.igPort = 8002;
	config.world = world;

	return std::make_unique<CigiClient>(config);
}

constexpr float epsilon = 1e-7f;

TEST_CASE("Entity is replicated from CIGI V3 packets")
{
	int cigiMajorVersion = 3;
	int cigiMinorVersion = 3;
	auto world = std::make_shared<DummyWorld>();
	auto client = CreateCigiClient(cigiMajorVersion, world);
	auto host = CreateCigiHost(cigiMajorVersion, cigiMinorVersion);

	// Create entity
	CigiIGCtrlV3 igCtrl;
	igCtrl.SetIGMode(CigiBaseIGCtrl::IGModeGrp::Operate);

	CigiEntityCtrlV3_3 entityCtrl;
	entityCtrl.SetEntityID(1);
	entityCtrl.SetEntityType(2);
	entityCtrl.SetEntityState(CigiBaseEntityCtrl::Active);
	entityCtrl.SetLat(3);
	entityCtrl.SetLon(4);
	entityCtrl.SetAlt(5);
	entityCtrl.SetRoll(6);
	entityCtrl.SetPitch(7);
	entityCtrl.SetYaw(8);

	host->send([&](auto& message) {
		message << igCtrl;
		message << entityCtrl;
	});

	REQUIRE(eventually([&] {
		client->update();
		return (world->entities.size() == 1);
	}));

	auto entity = dynamic_cast<DummyEntity*>(world->entities.begin()->get());
	REQUIRE(entity);
	CHECK(entity->type == 2);
	CHECK(entity->position.lat == Approx(3 * math::degToRadD()).epsilon(epsilon));
	CHECK(entity->position.lon == Approx(4 * math::degToRadD()).epsilon(epsilon));
	CHECK(entity->position.alt == 5);
	CHECK(entity->orientation.x == Approx(6 * math::degToRadD()).epsilon(epsilon));
	CHECK(entity->orientation.y == Approx(7 * math::degToRadD()).epsilon(epsilon));
	CHECK(entity->orientation.z == Approx(8 * math::degToRadD()).epsilon(epsilon));
	CHECK(entity->visible);

	// Update entity position
	entityCtrl.SetAlt(50);

	host->send([&](auto& message) {
		message << igCtrl;
		message << entityCtrl;
	});

	CHECK(eventually([&] {
		client->update();
		return (entity->position.alt == 50);
	}));

	// Hide entity
	entityCtrl.SetEntityState(CigiBaseEntityCtrl::Standby);

	host->send([&](auto& message) {
		message << igCtrl;
		message << entityCtrl;
	});

	CHECK(eventually([&] {
		client->update();
		return !entity->visible;
	}));
	CHECK(world->entities.size() == 1);

	// Destroy entity
	entityCtrl.SetEntityState(CigiBaseEntityCtrl::Destroyed);

	host->send([&](auto& message) {
		message << igCtrl;
		message << entityCtrl;
	});

	CHECK(eventually([&] {
		client->update();
		return (world->entities.size() == 0);
	}));
}

TEST_CASE("Camera is replicated from CIGI V3 packets")
{
	int cigiMajorVersion = 3;
	int cigiMinorVersion = 3;
	auto world = std::make_shared<DummyWorld>();
	auto client = CreateCigiClient(cigiMajorVersion, world);
	auto host = CreateCigiHost(cigiMajorVersion, cigiMinorVersion);

	CigiIGCtrlV3 igCtrl;
	igCtrl.SetIGMode(CigiBaseIGCtrl::IGModeGrp::Operate);

	// Create camera
	float halfHorizontalFov = 25;
	float halfVerticalFov = 20;

	CigiViewDefV3 viewDef;
	viewDef.SetViewID(1);
	viewDef.SetFOVBottom(halfVerticalFov);
	viewDef.SetFOVTop(halfVerticalFov);
	viewDef.SetFOVLeft(halfHorizontalFov);
	viewDef.SetFOVRight(halfHorizontalFov);

	host->send([&](auto& message) {
		message << igCtrl;
		message << viewDef;
	});

	REQUIRE(eventually([&] {
		client->update();
		return (world->cameras.size() == 1);
	}));

	auto camera = dynamic_cast<DummyCamera*>(world->cameras.begin()->get());
	REQUIRE(camera);
	CHECK(camera->horizontalFov == Approx(50.f * math::degToRadD()).epsilon(epsilon));
	CHECK(camera->verticalFov == Approx(40.f * math::degToRadD()).epsilon(epsilon));

	// Create entity and attach camera to it
	CigiEntityCtrlV3_3 entityCtrl;
	entityCtrl.SetEntityID(42);

	CigiViewCtrlV3 viewCtrl;
	viewCtrl.SetEntityID(42);
	viewCtrl.SetViewID(1);

	host->send([&](auto& message) {
		message << igCtrl;
		message << entityCtrl;
		message << viewCtrl;
	});

	REQUIRE(eventually([&] {
		client->update();
		return (camera->parent != nullptr);
	}));

	// Update field of view
	viewDef.SetFOVTop(10);
	viewDef.SetFOVBottom(10);

	host->send([&](auto& message) {
		message << igCtrl;
		message << viewDef;
	});

	CHECK(eventually([&] {
		client->update();
		return almostEqual(camera->verticalFov, 20.f * math::degToRadF(), epsilon);
	}));
}