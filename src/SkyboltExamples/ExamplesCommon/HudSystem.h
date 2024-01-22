#pragma once

#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltSim/Entity.h>
#include <SkyboltSim/System/System.h>

#include <SkyboltVis/Scene.h>
#include <SkyboltVis/Window/Window.h>

namespace skybolt {

class HeadingRibbonModel;
class HudHoverVelModel;
class PitchLadderModel;
class AltitudeBarModel;
class RollAngleRibbonModel;

class HudSystem : public sim::System, public sim::EntityListener
{
public:
	HudSystem(const osg::ref_ptr<osg::Camera>& parentCamera, const std::function<double()>& verticalFovProvider);
	~HudSystem() override;

	SKYBOLT_BEGIN_REGISTER_UPDATE_HANDLERS
		SKYBOLT_REGISTER_UPDATE_HANDLER(sim::UpdateStage::Attachments, updateState)
	SKYBOLT_END_REGISTER_UPDATE_HANDLERS

	void updateState();

	//! @param entity may be null
	void setEntity(sim::Entity* entity);

	void setEnabled(bool enabled) { mEnabled = enabled; }
	void setPitchLadderVisible(bool visible) { mPitchLadderVisible = visible; }

private: // EntityListener interface
	void onDestroy(sim::Entity* entity) override;

private:
	osg::ref_ptr<osg::Camera> mParentCamera;
	std::function<double()> mVerticalFovProvider;
	sim::Entity* mEntity = nullptr;
	bool mEnabled = true;
	bool mPitchLadderVisible = true;
	bool mInScene = false;
	osg::ref_ptr<VisHud> mHud;
	std::unique_ptr<HeadingRibbonModel> mHeadingRibbon;
	std::unique_ptr<HudHoverVelModel> mHoverVelModel;
	std::unique_ptr<PitchLadderModel> mPitchLadder;
	std::unique_ptr<AltitudeBarModel> mAltitudeBar;
	std::unique_ptr<RollAngleRibbonModel> mRollAngleRibbon;
};

} // namespace skybolt
