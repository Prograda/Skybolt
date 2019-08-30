/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DataModelRegistryFactory.h"
#include "Functions/FlowFunction.h"
#include "Nodes/AdditionNdm.h"
#include "Nodes/DataSeriesReaderNdm.h"
#include "Nodes/DataSeriesBuilderNdm.h"
#include "Nodes/DataSeriesLookupNdm.h"
#include "Nodes/DataSeriesPublisherNdm.h"
#include "Nodes/DisplayNdm.h"
#include "Nodes/FunctionNdm.h"
#include "Nodes/GeocentricOrientationNdm.h"
#include "Nodes/GetComponentNdm.h"
#include "Nodes/GetEntityNdm.h"
#include "Nodes/GetPropertyNdm.h"
#include "Nodes/JoystickNdm.h"
#include "Nodes/LatLonAltNdm.h"
#include "Nodes/LtpOrientationNdm.h"
#include "Nodes/MapSamplesNdm.h"
#include "Nodes/MultiplyNdm.h"
#include "Nodes/NodeContext.h"
#include "NodePropertiesModel.h"
#include "Nodes/Plot3dNdm.h"
#include "Nodes/PoseEntityNdm.h"
#include "Nodes/RerangeNdm.h"
#include "Nodes/Vector3Ndm.h"
#include "Nodes/TimeSourceNdm.h"

template<typename ModelType>
void registerModel(QtNodes::DataModelRegistry& registry, NodeContext* nodeContext, QString const &category = "Nodes")
{
	QtNodes::DataModelRegistry::RegistryItemCreator creator = [=]() { return std::make_unique<ModelType>(nodeContext); };
	registry.registerModel<ModelType>(std::move(creator), category);
}

std::shared_ptr<QtNodes::DataModelRegistry> registerDataModels(NodeContext* context)
{
	auto ret = std::make_shared<QtNodes::DataModelRegistry>();
	QtNodes::DataModelRegistry& registry = *ret;
	registerModel<AdditionNdm>(registry, context);
	registerModel<DataSeriesReaderNdm>(registry, context);
	registerModel<DataSeriesBuilderNdm>(registry, context);
	registerModel<DataSeriesLookupNdm>(registry, context);
	registerModel<DataSeriesPublisherNdm>(registry, context);
	registerModel<DisplayNdm>(registry, context);
	registerModel<GeocentricOrientationNdm>(registry, context);
	registerModel<GetComponentNdm>(registry, context);
	registerModel<GetEntityNdm>(registry, context);
	registerModel<GetPropertyNdm>(registry, context);
	registerModel<JoystickNdm>(registry, context);
	registerModel<LatLonAltNdm>(registry, context);
	registerModel<LtpOrientationNdm>(registry, context);
	registerModel<Plot3dNdm>(registry, context);
	registerModel<PoseEntityNdm>(registry, context);
	registerModel<MapSamplesNdm>(registry, context);
	registerModel<MultiplyNdm>(registry, context);
	registerModel<RerangeNdm>(registry, context);
	registerModel<Vector3Ndm>(registry, context);
	registerModel<TimeSourceNdm>(registry, context);
	return ret;
}