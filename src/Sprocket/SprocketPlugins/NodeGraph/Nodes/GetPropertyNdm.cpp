/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <pybind11/embed.h> // Include before Qt to avoid clash with slots macro
#include "GetPropertyNdm.h"
#include "NodeContext.h"
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Components/FuselageComponent.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <optional>

using namespace skybolt::sim;
namespace py = pybind11;
using namespace py::literals;

GetPropertyNdm::GetPropertyNdm(NodeContext* context)
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = { { EntityNodeData::typeId(), "entity" },{ StringNodeData::typeId(), "name" } };
	def->outputs = { { DoubleNodeData::typeId(), "value" } };
	setNodeDef(def);

	mConnections.push_back(context->timeSource->timeChanged.connect([this](double time) {SimpleNdm::eval(); }));
}

std::shared_ptr<QtNodes::NodeData> GetPropertyNdm::eval(const NodeDataVector& inputs, int outputIndex) const
{
	Entity* entity = getDataOrDefault<EntityNodeData>(inputs[0].get());
	if (entity)
	{
		const std::string& propertyName = getDataOrDefault<StringNodeData>(inputs[1].get());

		py::object pyObject = py::cast(static_cast<Entity*>(entity));

		auto locals = py::dict("object"_a = pyObject);
		pyObject = py::eval("locals()['object']." + propertyName, py::globals(), locals);
		double value = pyObject.cast<double>();

		return toNodeData<DoubleNodeData>(outputIndex, value);
	}
	return nullptr;
}
