/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0 // workaround for cigicl header errors with cpp17
#include <cigicl/CigiHostSession.h>
#include <cigicl/CigiEntityCtrlV3_3.h>
#include <cigicl/CigiIGCtrlV3.h>
#include <cigicl/CigiViewCtrlV3.h>
#include <cigicl/CigiViewDefV3.h>
#undef _HAS_STD_BYTE

#include <CigiComponent/UdpCommunicator.h>

#include <SkyboltCommon/Eventually.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <SkyboltCommon/NumericComparison.h>

#include <chrono>

using namespace skybolt;

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

int main(int argc, char *argv[])
{
	int cigiMajorVersion = 3;
	int cigiMinorVersion = 3;
	auto host = CreateCigiHost(cigiMajorVersion, cigiMinorVersion);

	CigiIGCtrlV3 igCtrl;
	igCtrl.SetIGMode(CigiBaseIGCtrl::IGModeGrp::Operate);

	// Create camera 1
	{
		float halfHorizontalFov = 25;
		float halfVerticalFov = 20;

		CigiViewDefV3 viewDef;
		viewDef.SetViewID(1);
		viewDef.SetFOVBottom(halfVerticalFov);
		viewDef.SetFOVTop(halfVerticalFov);
		viewDef.SetFOVLeft(halfHorizontalFov);
		viewDef.SetFOVRight(halfHorizontalFov);

		// Create entity and attach camera to it
		CigiEntityCtrlV3_3 entityCtrl;
		entityCtrl.SetEntityID(1);
		entityCtrl.SetEntityType(0);
		entityCtrl.SetEntityState(CigiBaseEntityCtrl::Active);
		entityCtrl.SetLat(53);
		entityCtrl.SetLon(0);
		entityCtrl.SetAlt(10000);
		entityCtrl.SetPitch(-10);
		
		CigiViewCtrlV3 viewCtrl;
		viewCtrl.SetEntityID(1);
		viewCtrl.SetViewID(1);

		host->send([&](auto& message) {
			message << igCtrl;
			message << entityCtrl;
			message << viewDef;
			message << viewCtrl;
		});
	}
	
	// Create camera 2
	{
		float halfHorizontalFov = 25;
		float halfVerticalFov = 20;

		CigiViewDefV3 viewDef;
		viewDef.SetViewID(2);
		viewDef.SetFOVBottom(halfVerticalFov);
		viewDef.SetFOVTop(halfVerticalFov);
		viewDef.SetFOVLeft(halfHorizontalFov);
		viewDef.SetFOVRight(halfHorizontalFov);

		// Create entity and attach camera to it
		CigiEntityCtrlV3_3 entityCtrl;
		entityCtrl.SetEntityID(2);
		entityCtrl.SetEntityType(0);
		entityCtrl.SetEntityState(CigiBaseEntityCtrl::Active);
		entityCtrl.SetLat(53);
		entityCtrl.SetLon(0);
		entityCtrl.SetAlt(10000);
		entityCtrl.SetPitch(-10);
		entityCtrl.SetYaw(45);
		
		CigiViewCtrlV3 viewCtrl;
		viewCtrl.SetEntityID(2);
		viewCtrl.SetViewID(2);

		host->send([&](auto& message) {
			message << igCtrl;
			message << entityCtrl;
			message << viewDef;
			message << viewCtrl;
		});
	}
	
	// Create entity
	{
		CigiEntityCtrlV3_3 entityCtrl;
		entityCtrl.SetEntityID(2);
		entityCtrl.SetEntityType(118);
		entityCtrl.SetEntityState(CigiBaseEntityCtrl::Active);
		entityCtrl.SetLat(53.0003);
		entityCtrl.SetLon(0.0003);
		entityCtrl.SetAlt(10000);

		host->send([&](auto& message) {
			message << igCtrl;
			message << entityCtrl;
		});
	}
}