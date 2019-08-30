/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "GetEntityNdm.h"
#include "NodeContext.h"
#include <SkyboltSim/Components/NameComponent.h>

using namespace skybolt;

GetEntityNdm::GetEntityNdm(NodeContext* context) :
	mNamedObjectRegistry(context->namedObjectRegistry),
	mWorld(context->simWorld)
{
	assert(mNamedObjectRegistry);
	assert(mWorld);

	mWorld->addListener(this);

	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = { { StringNodeData::typeId(), "name" } };
	def->outputs = { { EntityNodeData::typeId(), "entity" } };
	setNodeDef(def);
}

GetEntityNdm::~GetEntityNdm()
{
	mWorld->removeListener(this);
}

std::shared_ptr<QtNodes::NodeData> GetEntityNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	mCurrentEntityName = getDataOrDefault<StringNodeData>(inputs[0].get());

	sim::Entity* entity = mNamedObjectRegistry->getObjectByName(mCurrentEntityName);
	return toNodeData<EntityNodeData>(outputIndex, entity);
}

void GetEntityNdm::entityAdded(const sim::EntityPtr&  object)
{
	if (mCurrentEntityName == getName(*object))
	{
		SimpleNdm::eval();
	}
}

void GetEntityNdm::entityRemoved(const sim::EntityPtr& entity)
{
	// Update node output data so that it no longer points to the removed object
	if (mCurrentEntityName == getName(*entity))
	{
		SimpleNdm::eval();
	}
}
