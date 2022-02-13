/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SetOrientationContextAction.h"
#include "OrientationEditor.h"
#include "Sprocket/QDialogHelpers.h"
#include <SkyboltEngine/TemplateNameComponent.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <QComboBox>

using namespace skybolt;
using namespace skybolt::sim;

bool SetOrientationContextAction::handles(const Entity& entity) const
{
	auto position = getPosition(entity);
	auto orientation = getOrientation(entity);
	if (position && orientation)
	{
		return entity.getFirstComponent<TemplateNameComponent>() != nullptr;
	}
	return false;
}

void SetOrientationContextAction::execute(Entity& entity) const
{
	auto position = getPosition(entity);
	auto orientation = getOrientation(entity);
	assert(position);
	assert(orientation);

	sim::LatLon latLon = sim::toLatLon(toLatLonAlt(GeocentricPosition(*position)).position);

	OrientationEditor* editor = new OrientationEditor();
	editor->setOrientation(std::make_shared<GeocentricOrientation>(*orientation), latLon);
	auto dialog = createDialog(editor, "Set Orientation");

	if (dialog->exec() == QDialog::Accepted)
	{
		sim::OrientationPtr orientation = editor->getOrientation();
		setOrientation(entity, toGeocentric(*orientation, latLon).orientation);
	}
}
