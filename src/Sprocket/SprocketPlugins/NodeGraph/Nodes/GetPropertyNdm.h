/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimpleNdm.h"
#include <boost/signals2.hpp>

class GetPropertyNdm : public SimpleNdm
{
public:
	GetPropertyNdm(NodeContext* context);

	static QString Name() { return "GetProperty"; }

	std::shared_ptr<QtNodes::NodeData> eval(const NodeDataVector& inputs, int outputIndex) const override;

private:
	std::vector<boost::signals2::scoped_connection> mConnections;
};
