/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING // Ignore boost/asio warnings
#include "UdpCommunicator.h"

#ifdef WIN32
#define _WIN32_WINNT 0x0601 // Boost/asio requires the windows platform target to be set
#endif
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class UdpCommunicatorImpl
{
public:
	UdpCommunicatorImpl(const UdpCommunicatorConfig& config)
	{
		udp::resolver resolver(mService);
		{
			udp::resolver::query query(udp::v4(), config.remoteAddress, std::to_string(config.remotePort));
			mEndpoint = *resolver.resolve(query);
		}

		mSocket = std::make_unique<udp::socket>(mService);
		mSocket->open(udp::v4());

		mReceiveSocket = std::make_unique<udp::socket>(mService);
		mReceiveSocket->open(udp::v4());
		mReceiveSocket->non_blocking(true);

		{
			udp::resolver::query query(udp::v4(), config.localAddress, std::to_string(config.localPort));
			auto endpoint = *resolver.resolve(query);
			mReceiveSocket->bind(endpoint);
		}
	}

	~UdpCommunicatorImpl()
	{
		mReceiveSocket->close();
		mSocket->close();
	}

	size_t receive(unsigned char& data, size_t sizeBytes)
	{
		udp::endpoint senderEndpoint;
		if (mReceiveSocket->available())
		{
			size_t len = mReceiveSocket->receive_from(boost::asio::buffer(&data, sizeBytes), senderEndpoint);
			if (mEndpoint.address() == senderEndpoint.address())
			{
				return len;
			}
		}

		return 0;
	}

	void send(unsigned char& data, size_t sizeBytes)
	{
		mSocket->send_to(boost::asio::buffer(&data, sizeBytes), mEndpoint);
	}

private:
	boost::asio::io_service mService;
	std::unique_ptr<udp::socket> mSocket;
	std::unique_ptr<udp::socket> mReceiveSocket;
	udp::endpoint mEndpoint;
};

UdpCommunicator::UdpCommunicator(const UdpCommunicatorConfig& config) :
	mImpl(std::make_unique<UdpCommunicatorImpl>(config))
{
}

UdpCommunicator::~UdpCommunicator()
{
}

size_t UdpCommunicator::receive(unsigned char& data, size_t sizeBytes)
{
	return mImpl->receive(data, sizeBytes);
}

void UdpCommunicator::send(unsigned char& data, size_t sizeBytes)
{
	mImpl->send(data, sizeBytes);
}
