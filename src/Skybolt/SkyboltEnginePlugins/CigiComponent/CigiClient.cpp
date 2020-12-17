/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CigiClient.h"
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/Math/MathUtility.h>

#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0 // workaround for cigicl header errors with cpp17
#include <cigicl/CigiEntityCtrlV3_3.h>
#include <cigicl/CigiEntityCtrlV4.h>
#include <cigicl/CigiEntityPositionCtrlV4.h>
#include <cigicl/CigiIGSession.h>
#include <cigicl/CigiIncomingMsg.h>
#include <cigicl/CigiSOFV3.h>
#include <cigicl/CigiViewCtrlV3.h>
#include <cigicl/CigiViewCtrlV4.h>
#include <cigicl/CigiViewDefV3.h>
#include <cigicl/CigiViewDefV4.h>
#undef _HAS_STD_BYTE

#include <boost/log/trivial.hpp>

namespace skybolt {

class CigiBaseEventProcessorI : public CigiBaseEventProcessor
{
public:
	CigiBaseEventProcessorI(const ProcessPacketFunction& function) :
		function(function)
	{}

	void OnPacketReceived(CigiBasePacket* packet) override
	{
		function(*packet);
	}

	ProcessPacketFunction function;
};

CigiClient::CigiClient(const CigiClientConfig& config) :
	mWorld(config.world),
	mReceiveBuffer(mMaxReceiveBufferSizeBytes)
{
	UdpCommunicatorConfig socketConfig;
	socketConfig.localAddress = "localhost";
	socketConfig.localPort = config.igPort;
	socketConfig.remoteAddress = config.host;
	socketConfig.remotePort = config.hostPort;
	mSocket = std::make_unique<UdpCommunicator>(socketConfig);

	int minorVersion = (config.cigiMajorVersion == 3) ? 3 : 0;

	mOutgoingSession = std::make_unique<CigiIGSession>();
	mOutgoingSession->SetCigiVersion(CigiVersionID(config.cigiMajorVersion, minorVersion));
	mOutgoingSession->SetSynchronous(true);

	mIncomingSession = std::make_unique<CigiIGSession>();
	mIncomingSession->SetCigiVersion(CigiVersionID(config.cigiMajorVersion, minorVersion));
	mIncomingSession->SetSynchronous(true);
		
	CigiIncomingMsg &incomingMessage = mIncomingSession->GetIncomingMsgMgr();

	incomingMessage.SetReaderCigiVersion(config.cigiMajorVersion, minorVersion);
	incomingMessage.UsingIteration(false);

	if (config.cigiMajorVersion == 3)
	{
		registerEventProcessor<CigiEntityCtrlV3_3>(CIGI_ENTITY_CTRL_PACKET_ID_V3_3);
		// These V3 packets are forward compatible with V4
		registerEventProcessor<CigiViewCtrlV4>(CIGI_VIEW_CTRL_PACKET_ID_V3);
		registerEventProcessor<CigiViewDefV4>(CIGI_VIEW_DEF_PACKET_ID_V3);
	}
	else if (config.cigiMajorVersion == 4)
	{
		registerEventProcessor<CigiEntityCtrlV4>(CIGI_ENTITY_CTRL_PACKET_ID_V4);
		registerEventProcessor<CigiEntityPositionCtrlV4>(CIGI_ENTITY_POSITION_CTRL_PACKET_ID_V4);
		registerEventProcessor<CigiViewCtrlV4>(CIGI_VIEW_CTRL_PACKET_ID_V4);
		registerEventProcessor<CigiViewDefV4>(CIGI_VIEW_DEF_PACKET_ID_V4);
	}
	else
	{
		throw std::runtime_error("Unsupported CIGI major version: " + config.cigiMajorVersion);
	}

// TODO: Maybe remove the multithreading implementation.
// It's currently disabled because UdpCommunicator doesn't seem to work when sending and receiving on different threads.
// The receive buffer seems to gets backed up.
#ifdef MULTI_THREADED_CIGI_RECEIVER
	mReceiverThread = std::thread([this] {
		while (!mTerminateReceiverThread)
		{
			processIncomingMessages();

			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1ms);
		}
	});
#endif
}

CigiClient::~CigiClient()
{
	resetWorld();

	mTerminateReceiverThread = true;
#ifdef MULTI_THREADED_CIGI_RECEIVER
	mReceiverThread.join();
#endif
}

void CigiClient::sendFrame()
{
	CigiSOFV3 sof;
	sof.SetDatabaseID(0);
	sof.SetEarthRefModel(CigiBaseSOF::WGS84);
	sof.SetFrameCntr(mFrameCounter);
	sof.SetIGMode(CigiBaseSOF::Operate);
	sof.SetIGStatus(0);

	try
	{
		CigiOutgoingMsg &outgoingMessage = mOutgoingSession->GetOutgoingMsgMgr();
		outgoingMessage.BeginMsg();
		outgoingMessage << sof;

		Cigi_uint8* message = 0;
		int length = 0;
		outgoingMessage.PackageMsg(&message, length);

		mSocket->send(*message, length);

		outgoingMessage.FreeMsg();
	}
	catch (const std::exception& e)
	{
		BOOST_LOG_TRIVIAL(error) << "CigiClient error: " << e.what();
	}

	++mFrameCounter;
}

template <class BaseT>
class TypeDelegator
{
public:
	static TypeDelegator<BaseT> of(const BaseT* base)
	{
		return TypeDelegator<BaseT>(base);
	}

	TypeDelegator(const BaseT* base) : mBase(base) {}

	template <class DerivedT>
	TypeDelegator<BaseT>& with(const std::function<void(const DerivedT&)>& fn)
	{
		if (!mEvaluated)
		{
			auto derived = dynamic_cast<const DerivedT*>(mBase);
			if (derived)
			{
				mEvaluated = true;
				fn(*derived);
			}
		}

		return *this;
	}

private:
	const BaseT* mBase;
	bool mEvaluated = false;
};

void CigiClient::processEntityCtrlV4(const CigiEntityCtrlV4& packet)
{
	if (packet.GetEntityState() == CigiBaseEntityCtrl::Active || packet.GetEntityState() == CigiBaseEntityCtrl::Standby)
	{
		CigiEntityPtr entity;
		int id = packet.GetEntityID();
		auto it = mEntities.find(id);
		if (it == mEntities.end())
		{
			int typeId = packet.GetEntityType();
			entity = mWorld->createEntity(typeId);
			if (entity)
			{
				mEntities.insert(std::make_pair(id, entity)).first;
			}
		}
		else
		{
			entity = it->second;
		}
		entity->setVisible(packet.GetEntityState() == CigiBaseEntityCtrl::Active);
	}
	else if (packet.GetEntityState() == CigiBaseEntityCtrl::Remove || packet.GetEntityState() == CigiBaseEntityCtrl::Destroyed)
	{
		auto it = mEntities.find(packet.GetEntityID());
		if (it != mEntities.end())
		{
			mWorld->destroyEntity(it->second);
			mEntities.erase(it);
		}
	}
}

void CigiClient::processEntityPositionCtrlV4(const CigiEntityPositionCtrlV4& packet)
{
	int id = packet.GetEntityID();
	auto it = mEntities.find(id);
	if (it != mEntities.end())
	{
		CigiEntityPtr entity = it->second;
		entity->setPosition(sim::LatLonAlt(packet.GetLat() * math::degToRadD(), packet.GetLon() * math::degToRadD(), packet.GetAlt()));
		entity->setOrientation(sim::Vector3(packet.GetRoll() * math::degToRadD(), packet.GetPitch() * math::degToRadD(), packet.GetYaw() * math::degToRadD()));
	}
}

void CigiClient::update()
{
#ifndef MULTI_THREADED_CIGI_RECEIVER
	processIncomingMessages();
#endif
	std::vector<CigiBasePacketPtr> packetQueue;
	{
		std::lock_guard<std::mutex> lock(mPacketQueueMutex);
		std::swap(packetQueue, mPacketQueue);
	}

	for (const auto& basePacket : packetQueue)
	{
		TypeDelegator<CigiBasePacket>::of(basePacket.get())
		.with<CigiEntityCtrlV3_3>([this] (const auto& packet) {
			CigiEntityCtrlV4 ctrlPacket;
			ctrlPacket.SetEntityID(packet.GetEntityID());
			ctrlPacket.SetEntityState(packet.GetEntityState());
			ctrlPacket.SetEntityType(packet.GetEntityType());
			processEntityCtrlV4(ctrlPacket);

			constexpr bool boundsCheck = false;
			CigiEntityPositionCtrlV4 positionPacket;
			positionPacket.SetEntityID(packet.GetEntityID());
			positionPacket.SetLat(packet.GetLat(), boundsCheck);
			positionPacket.SetLon(packet.GetLon(), boundsCheck);
			positionPacket.SetAlt(packet.GetAlt(), boundsCheck);
			positionPacket.SetYaw(packet.GetYaw(), boundsCheck);
			positionPacket.SetPitch(packet.GetPitch(), boundsCheck);
			positionPacket.SetRoll(packet.GetRoll(), boundsCheck);
			processEntityPositionCtrlV4(positionPacket);
		})
		.with<CigiEntityCtrlV4>([this](const auto& packet) {
			processEntityCtrlV4(packet);
		})
		.with<CigiEntityPositionCtrlV4>([this](const auto& packet) {
			processEntityPositionCtrlV4(packet);
		})
		.with<CigiViewCtrlV4>([this](const auto& packet) {
			auto camera = skybolt::findOptional(mCameras, (int)packet.GetViewID());
			if (camera)
			{
				auto entity = skybolt::findOptional(mEntities, (int)packet.GetEntityID());
				if (entity)
				{
					(*camera)->setParent(*entity);
				}
				else
				{
					(*camera)->setParent(nullptr);
				}
			}
		})
		.with<CigiViewDefV4>([this](const auto& packet) {
			int id = packet.GetViewID();
			std::shared_ptr<CigiCamera> camera;
			auto optionalCamera = skybolt::findOptional(mCameras, id);
			if (optionalCamera)
			{
				camera = *optionalCamera;
			}
			else
			{
				camera = mWorld->createCamera();
				mCameras[id] = camera;
			}
			camera->setHorizontalFieldOfView((packet.GetFOVLeft() + packet.GetFOVRight()) * math::degToRadD());
			camera->setVerticalFieldOfView((packet.GetFOVTop() + packet.GetFOVBottom()) * math::degToRadD());
		});
	}
}

void CigiClient::resetWorld()
{
	for (const auto& camera : mCameras)
	{
		mWorld->destroyCamera(camera.second);
	}

	for (const auto& entity : mEntities)
	{
		mWorld->destroyEntity(entity.second);
	}
}

void CigiClient::processIncomingMessages()
{
	try
	{
		size_t numBytesRead = mSocket->receive(*mReceiveBuffer.data(), mMaxReceiveBufferSizeBytes);
		if (numBytesRead > 0)
		{
			CigiIncomingMsg &incomingMessage = mIncomingSession->GetIncomingMsgMgr();
			incomingMessage.ProcessIncomingMsg(mReceiveBuffer.data(), (int)numBytesRead);
		}
	}
	catch (const std::exception& e)
	{
		BOOST_LOG_TRIVIAL(error) << "CigiClient error: " << e.what();
	}
}

void CigiClient::registerEventProcessor(int eventId, const ProcessPacketFunction& function)
{
	auto processor = std::make_shared<CigiBaseEventProcessorI>(function);
	mCigiBaseEventProcessors.push_back(processor);
	mIncomingSession->GetIncomingMsgMgr().RegisterEventProcessor(eventId, processor.get());
}

} // namespace skybolt