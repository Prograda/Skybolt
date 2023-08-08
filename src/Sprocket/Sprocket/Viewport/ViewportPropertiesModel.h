#pragma once

#include "Sprocket/Property/PropertyModel.h"
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltVis/SkyboltVisFwd.h>

class ViewportPropertiesModel : public PropertiesModel
{
public:
	ViewportPropertiesModel(skybolt::vis::Scene* scene, skybolt::sim::CameraComponent* camera);

private:
	skybolt::vis::Scene* mScene;
	VariantPropertyPtr mFov;
	VariantPropertyPtr mAmbientLight;
};