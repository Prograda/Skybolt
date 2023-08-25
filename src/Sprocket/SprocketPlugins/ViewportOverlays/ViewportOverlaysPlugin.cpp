/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VisSelectionIcons.h"
#include <Sprocket/EditorPlugin.h>
#include <Sprocket/QtUtil/QtMenuUtil.h>
#include <Sprocket/Scenario/EntityObjectType.h>
#include <Sprocket/Scenario/ScenarioSelectionModel.h>

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltEngine/SimVisBinding/ForcesVisBinding.h>
#include <SkyboltEngine/SimVisBinding/GeocentricToNedConverter.h>
#include <SkyboltEngine/SimVisBinding/SimVisSystem.h>
#include <SkyboltEngine/SimVisBinding/VisNameLabels.h>
#include <SkyboltEngine/SimVisBinding/VisOrbits.h>
#include <SkyboltVis/Scene.h>
#include <SkyboltVis/Renderable/Arrows.h>

#include <assert.h>
#include <boost/config.hpp>
#include <boost/dll/alias.hpp>
#include <osg/Texture2D>
#include <QMainWindow>
#include <QMenu>

using namespace skybolt;

static osg::ref_ptr<osg::Texture> createEntitySelectionIcon(int width = 64)
{
	int height = width;
	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
	std::uint8_t* p = image->data();
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
#ifdef ENTITY_SELECTION_ICON_BOX
			bool edge = (x == 0 || y == 0 || x == (width - 1) || y == (height - 1));
			int alpha = edge ? 255 : 0;
#else // circle
			int rx = std::abs(x - width/2);
			int ry = std::abs(y - width/2);
			float r = glm::length(glm::vec2(float(rx), float(ry)));
			
			float rampUpStart = width/2 - 3;
			float rampUpEnd = rampUpStart+1;
			float rampDownStart = rampUpStart+2;
			float rampDownEnd = rampUpStart+3;
			int alpha = int(255.f * std::max(0.f, (glm::smoothstep(rampUpStart, rampUpEnd, r)) - std::max(0.f, glm::smoothstep(rampDownStart, rampDownEnd, r))));
#endif
			*p++ = 255;
			*p++ = 255;
			*p++ = 255;
			*p++ = alpha;
		}
	}

	return new osg::Texture2D(image);
}

class ViewportOverlaysSystem : public sim::System
{
public:
	ViewportOverlaysSystem(std::function<void()> updater) : mUpdater(std::move(updater)) {}

	void updatePostDynamics(const StepArgs& args) override
	{
		mUpdater();
	}

private:
	 std::function<void()> mUpdater;
};

class ViewportOverlaysPlugin : public EditorPlugin
{
public:
	ViewportOverlaysPlugin(const EditorPluginConfig& config) :
		mEngineRoot(config.engineRoot)
	{
		// Handle selection events
		QObject::connect(config.selectionModel, &ScenarioSelectionModel::selectionChanged, [this] (const SelectedScenarioObjects& selected, const SelectedScenarioObjects& deselected) {
			selectionChanged(selected);
		});

		// Create ViewportOverlaysSystem
		{
			auto simVisSystem = sim::findSystem<SimVisSystem>(*mEngineRoot->systemRegistry);
			if (!simVisSystem)
			{
				throw std::runtime_error("Could not find SimVisSystem");
			}
			mCoordinateConverter = &simVisSystem->getCoordinateConverter();

			auto system = std::make_shared<ViewportOverlaysSystem>([this] {
				update();
			});
			mEngineRoot->systemRegistry->push_back(system);
		}

		// Create overlay visuals
		auto hudGroup = mEngineRoot->scene->getBucketGroup(vis::Scene::Bucket::Hud);

		mVisSelectionIcons = new VisSelectionIcons(mEngineRoot->programs, createEntitySelectionIcon());
		hudGroup->addChild(mVisSelectionIcons);

		sim::World* world = &mEngineRoot->scenario->world;

		mVisNameLabels = std::make_unique<VisNameLabels>(world, hudGroup, mEngineRoot->programs);

		osg::ref_ptr<osg::Program> unlitColoredProgram = mEngineRoot->programs.getRequiredProgram("unlitColored");

		{
			vis::Polyline::Params params;
			params.program = unlitColoredProgram;
			mVisOrbits.reset(new VisOrbits(world, hudGroup, params, [scenario = mEngineRoot->scenario.get()] {
				return scenario->startJulianDate + scenario->timeSource.getTime();
			}));
		}
	
		{
			vis::Arrows::Params params;
			params.program = unlitColoredProgram;

			auto arrows = std::make_shared<vis::Arrows>(params);
			mEngineRoot->scene->addObject(arrows);
			mForcesVisBinding.reset(new ForcesVisBinding(world, arrows));
		}
	}

	static EntityVisibilityPredicateSetter toEntityVisibilityPredicateSetter(EntityVisibilityFilterable* filterable)
	{
		return [filterable] (EntityVisibilityPredicate p) { filterable->setEntityVisibilityPredicate(std::move(p)); };
	}

	EntityVisibilityLayerMap getEntityVisibilityLayers() const override
	{
		return {
			{"Labels", toEntityVisibilityPredicateSetter(mVisNameLabels.get())},
			{"Forces", toEntityVisibilityPredicateSetter(mForcesVisBinding.get())},
			{"Orbits", toEntityVisibilityPredicateSetter(mVisOrbits.get())}
		};
	}

	void selectionChanged(const SelectedScenarioObjects& selected)
	{
		mVisSelectionIcons->setSelection(selected);
	}

	static glm::dvec3 toGlm(const osg::Vec3d& v)
	{
		return glm::dvec3(v.x(), v.y(), v.z());
	}

	void update()
	{
		mVisSelectionIcons->syncVis(*mCoordinateConverter);
		mVisNameLabels->syncVis(*mCoordinateConverter);
		mVisOrbits->syncVis(*mCoordinateConverter);
		mForcesVisBinding->syncVis(*mCoordinateConverter);
	}

private:
	EngineRoot* mEngineRoot;
	const GeocentricToNedConverter* mCoordinateConverter;
	osg::ref_ptr<skybolt::VisSelectionIcons> mVisSelectionIcons;
	std::unique_ptr<skybolt::VisNameLabels> mVisNameLabels;
	std::unique_ptr<skybolt::VisOrbits> mVisOrbits;
	std::unique_ptr<skybolt::ForcesVisBinding> mForcesVisBinding;

	sim::EntityId mSelectedEntityId;
};

namespace plugins {

	std::shared_ptr<EditorPlugin> createEditorPlugin(const EditorPluginConfig& config)
	{
		return std::make_shared<ViewportOverlaysPlugin>(config);
	}

	BOOST_DLL_ALIAS(
		plugins::createEditorPlugin,
		createEditorPlugin
	)
}
