#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltReflect/SkyboltReflectFwd.h>
#include <SkyboltWidgets/Property/QtProperty.h>
#include <SkyboltWidgets/Property/PropertyEditorWidgetFactory.h>
#include <SkyboltWidgets/Property/QtPropertyReflection.h>
#include <SkyboltSim/Entity.h>

class EntityPropertiesModel : public skybolt::PropertiesModel, public skybolt::sim::EntityListener
{
public:
	EntityPropertiesModel(skybolt::refl::TypeRegistry* typeRegistry, const skybolt::ReflTypePropertyFactoryMapPtr& factoryMap, skybolt::sim::Entity* entity = nullptr);
	~EntityPropertiesModel() override;

	void setEntity(skybolt::sim::Entity* entity);

private:
	void onDestroy(skybolt::sim::Entity* entity) override;

private:
	skybolt::refl::TypeRegistry* mTypeRegistry;
	skybolt::ReflTypePropertyFactoryMapPtr mReflTypePropertyFactoryMap;
	skybolt::sim::Entity* mEntity;
	bool mCurrentlyUpdating = false;
};
