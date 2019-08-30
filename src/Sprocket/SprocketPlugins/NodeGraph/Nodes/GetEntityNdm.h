/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimpleNdm.h"
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltSim/World.h>
#include <SkyboltSim/Components/NameComponent.h>

class GetEntityNdm : public SimpleNdm, public skybolt::sim::WorldListener
{
public:
	GetEntityNdm(NodeContext* context);
	~GetEntityNdm();

	static QString Name() { return "GetEntity"; }

	virtual std::shared_ptr<QtNodes::NodeData> eval(const NodeDataVector& inputs, int outputIndex) const;

private:
	void entityAdded(const skybolt::sim::EntityPtr& entity) override;
	void entityRemoved(const skybolt::sim::EntityPtr& entity) override;

private:
	const skybolt::sim::NamedObjectRegistry* mNamedObjectRegistry;
	mutable std::string mCurrentEntityName;
	skybolt::sim::World* mWorld;
};
