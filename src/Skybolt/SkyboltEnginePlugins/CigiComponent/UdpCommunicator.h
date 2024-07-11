/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <string>

struct UdpCommunicatorConfig
{
	std::string remoteAddress;
	int remotePort;
	std::string localAddress;
	int localPort;
};

//! @ThreadSafe
class UdpCommunicator
{
public:
	UdpCommunicator(const UdpCommunicatorConfig& config);
	~UdpCommunicator();

	size_t receive(unsigned char& data, size_t sizeBytes); //!< Returns number of bytes read
	void send(unsigned char& data, size_t sizeBytes);

private:
	std::unique_ptr<class UdpCommunicatorImpl> mImpl;
};