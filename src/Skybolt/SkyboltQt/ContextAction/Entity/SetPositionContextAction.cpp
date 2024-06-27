/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SetPositionContextAction.h"
#include "SkyboltQt/QtUtil/QtDialogUtil.h"
#include "Widgets/PositionEditor.h"
#include <SkyboltEngine/Components/TemplateNameComponent.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltCommon/Math/MathUtility.h>

#include <QComboBox>

using namespace skybolt;
using namespace skybolt::sim;

bool SetPositionContextAction::handles(const Entity& entity) const
{
	auto position = getPosition(entity);
	if (position)
	{
		return entity.getFirstComponent<TemplateNameComponent>() != nullptr;
	}
	return false;
}

void SetPositionContextAction::execute(Entity& entity) const
{
	auto position = getPosition(entity);
	assert(position);

	PositionEditor* editor = new PositionEditor();
	editor->setPosition(GeocentricPosition(*position));
	auto dialog = createDialogModal(editor, "Set Position");

	if (dialog->exec() == QDialog::Accepted)
	{
		sim::PositionPtr position = editor->getPosition();
		setPosition(entity, toGeocentric(*position).position);
	}
}
