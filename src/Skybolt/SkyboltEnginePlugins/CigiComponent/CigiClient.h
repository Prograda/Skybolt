/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Spatial/LatLonAlt.h>
#include <SkyboltSim/SimMath.h>

#include "UdpCommunicator.h"

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <thread>

class CigiBasePacket;
class CigiEntityCtrlV4;
class CigiEntityPositionCtrlV4;
class CigiIGSession;

namespace skybolt {

class CigiEntity
{
public:
	virtual ~CigiEntity() {}
	virtual void setPosition(const sim::LatLonAlt& position) = 0;
	virtual void setOrientation(const sim::Vector3& ypr) = 0;
	virtual void setVisible(bool visibile) = 0;
};

typedef std::shared_ptr<CigiEntity> CigiEntityPtr;

class CigiCamera
{
public:
	virtual ~CigiCamera() {}
	virtual void setParent(const CigiEntityPtr& parent) = 0;
	virtual void setPositionOffset(const sim::Vector3& position) = 0;
	virtual void setOrientationOffset(const sim::Vector3& rpy) = 0;
	virtual void setHorizontalFieldOfView(float fov) = 0; //!< Angle in radians
	virtual void setVerticalFieldOfView(float fov) = 0; //!< Angle in radians
};

typedef std::shared_ptr<CigiCamera> CigiCameraPtr;

class CigiWorld
{
public:
	virtual ~CigiWorld() {}
	virtual CigiEntityPtr createEntity(int typeId) = 0;
	virtual void destroyEntity(const CigiEntityPtr& entity) = 0;
	
	virtual CigiCameraPtr createCamera() = 0;
	virtual void destroyCamera(const CigiCameraPtr& camera) = 0;
};

typedef std::shared_ptr<CigiWorld> CigiWorldPtr;

struct CigiClientConfig
{
	std::string host = "127.0.0.1";
	int hostPort = 8001;
	int igPort = 8002;
	int cigiMajorVersion = 3;
	CigiWorldPtr world;
};

typedef std::function<void(const CigiBasePacket& packet)> ProcessPacketFunction;

class CigiClient
{
public:
	CigiClient(const CigiClientConfig& config);
	~CigiClient();

	void sendFrame();

	void update();

private:
	void resetWorld();

	void processIncomingMessages();

	void registerEventProcessor(int eventId, const ProcessPacketFunction& function);
	
	template <typename T>
	void registerEventProcessor(int eventId)
	{
		registerEventProcessor(eventId, [this](const CigiBasePacket& packet) {
			const T* derivedPacket = static_cast<const T*>(&packet);
			std::lock_guard<std::mutex> lock(mPacketQueueMutex);
			mPacketQueue.push_back(std::make_shared<T>(*derivedPacket));
		});
	}

	void processEntityCtrlV4(const CigiEntityCtrlV4& packet);
	void processEntityPositionCtrlV4(const CigiEntityPositionCtrlV4& packet);

private:
	// Main thread
	CigiWorldPtr mWorld;
	std::unique_ptr<CigiIGSession> mOutgoingSession;
	int mFrameCounter = 0;
	std::map<int, CigiCameraPtr> mCameras;
	std::map<int, CigiEntityPtr> mEntities;

	// Receiver thread
	static const int mMaxReceiveBufferSizeBytes = 32768;
	std::vector<unsigned char> mReceiveBuffer;
	std::unique_ptr<CigiIGSession> mIncomingSession;
	std::thread mReceiverThread;
	std::atomic_bool mTerminateReceiverThread = false;
	std::vector<std::shared_ptr<class CigiBaseEventProcessorI>> mCigiBaseEventProcessors;

	// Shared between main thread and receiver thread
	typedef std::shared_ptr<CigiBasePacket> CigiBasePacketPtr;
	std::vector<CigiBasePacketPtr> mPacketQueue;
	std::mutex mPacketQueueMutex;

	std::unique_ptr<UdpCommunicator> mSocket; //!< @ThreadSafe
};

} // namespace skybolt