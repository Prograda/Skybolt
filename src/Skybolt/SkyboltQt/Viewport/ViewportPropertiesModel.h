/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/Property/PropertyModel.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltVis/SkyboltVisFwd.h>

class ViewportPropertiesModel : public PropertiesModel
{
public:
	ViewportPropertiesModel(skybolt::vis::Scene* scene, skybolt::sim::CameraComponent* camera);

private:
	skybolt::vis::Scene* mScene;
	QtPropertyPtr mFov;
	QtPropertyPtr mAmbientLight;
};