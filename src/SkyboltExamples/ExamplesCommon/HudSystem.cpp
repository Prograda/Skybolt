#include "HudSystem.h"

#include <AircraftHud/HeadingRibbonModel.h>
#include <AircraftHud/HudHoverVelModel.h>
#include <AircraftHud/PitchLadderModel.h>
#include <AircraftHud/AltitudeBarModel.h>
#include <AircraftHud/RollAngleRibbonModel.h>

#include <SkyboltSim/Components/ControlInputsComponent.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Components/Node.h>
#include <SkyboltSim/Spatial/Orientation.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltEngine/VisHud.h>
#include <SkyboltCommon/Math/MathUtility.h>

using namespace skybolt;
using namespace skybolt::sim;
using namespace skybolt::vis;

float pitchLadderAngleIncrement = 10.f * math::degToRadF();

HudSystem::HudSystem(const osg::ref_ptr<osg::Camera>& parentCamera, const std::function<double()>& verticalFovProvider) :
	mParentCamera(parentCamera),
	mVerticalFovProvider(verticalFovProvider)
{
	assert(mParentCamera);

	mHud = osg::ref_ptr<VisHud>(new VisHud());
	mHud->setColor(osg::Vec4f(0, 1, 0, 1));

	{
		HeadingRibbonModel::Parameters params;
		params.makeDefault();
		params.center.y = 0.7;

		mHeadingRibbon = std::make_unique<HeadingRibbonModel>(mHud.get(), params);
	}

	mHoverVelModel = std::make_unique<HudHoverVelModel>(mHud.get(), 0.05, 0.05);

	{
		PitchLadderModel::Parameters params;
		params.makeDefault();
		params.pitchAngleIncrement = pitchLadderAngleIncrement;
		mPitchLadder = std::make_unique<PitchLadderModel>(mHud.get(), params);
	}

	AltitudeBarModel::Parameters altitudeBarParams;
	altitudeBarParams.makeDefault();
	mAltitudeBar = std::make_unique<AltitudeBarModel>(mHud.get(), altitudeBarParams);

	RollAngleRibbonModel::Parameters rollAngleRibbonParams;
	rollAngleRibbonParams.makeDefault();
	mRollAngleRibbon = std::make_unique<RollAngleRibbonModel>(mHud.get(), rollAngleRibbonParams);
}

HudSystem::~HudSystem()
{
	setEntity(nullptr);
}

const double metersToFeet = 3.28084;
const double metersPerSecToKnots = 1.94384;

static std::string toIntString(double v)
{
	return std::to_string(int(std::round(v)));
}

static void drawVee(VisHud& hud, const glm::vec2& position)
{
	double halfWidth = 0.02;
	double height = 0.02;
	hud.drawLine(position + glm::vec2(-halfWidth * 2, 0), position + glm::vec2(-halfWidth, 0));
	hud.drawLine(position + glm::vec2(-halfWidth, 0), position + glm::vec2(0, -height));
	hud.drawLine(position + glm::vec2(halfWidth, 0), position + glm::vec2(0, -height));
	hud.drawLine(position + glm::vec2(halfWidth * 2, 0), position + glm::vec2(halfWidth, 0));
}

static void drawPlus(VisHud& hud, const glm::vec2& position, float size)
{
	double halfSize = size * 0.5;
	hud.drawLine(position + glm::vec2(-halfSize, 0), position + glm::vec2(halfSize, 0));
	hud.drawLine(position + glm::vec2(0, -halfSize), position + glm::vec2(0, halfSize));
}

static void drawBoxOutline(VisHud& hud, const glm::vec2& position, float width, float height)
{
	float halfWidth = width * 0.5;
	float halfHeight = height * 0.5;
	hud.drawLine(position + glm::vec2(-halfWidth, -halfHeight), position + glm::vec2(halfWidth, -halfHeight));
	hud.drawLine(position + glm::vec2(halfWidth, -halfHeight), position + glm::vec2(halfWidth, halfHeight));
	hud.drawLine(position + glm::vec2(halfWidth, halfHeight), position + glm::vec2(-halfWidth, halfHeight));
	hud.drawLine(position + glm::vec2(-halfWidth, halfHeight), position + glm::vec2(-halfWidth, -halfHeight));
}

float controlTickSize = 0.02;

static void drawControl2d(VisHud& hud, const glm::vec2& position, float size, const glm::vec2& controlValue)
{
	float halfSize = size * 0.5f;
	drawBoxOutline(hud, position, size, size);
	drawPlus(hud, position + controlValue * halfSize, controlTickSize);
}

static void drawHSlider(VisHud& hud, const glm::vec2& position, float width, const float& controlValue)
{
	float halfWidth = width * 0.5;
	hud.drawLine(position - glm::vec2(halfWidth, 0), position + glm::vec2(halfWidth, 0));

	double x = width * controlValue - halfWidth;
	hud.drawLine(position + glm::vec2(x, -controlTickSize * 0.5), position + glm::vec2(x, controlTickSize * 0.5));
}

static void drawVSlider(VisHud& hud, const glm::vec2& position, float height, const float& controlValue)
{
	float halfHeight = height * 0.5;
	hud.drawLine(position + glm::vec2(0, -halfHeight), position + glm::vec2(0, halfHeight));

	double y = height * controlValue - halfHeight;
	hud.drawLine(position + glm::vec2(-controlTickSize * 0.5, y), position + glm::vec2(controlTickSize * 0.5, y));
}

static void drawControlInputs(VisHud& hud, const glm::vec2& position, const ControlInputsComponent& controlInputsComponents)
{
	float halfTickWidth = 0.01f;
	float size = 0.1f;
	float margin = 0.02f;

	auto stick = controlInputsComponents.get<glm::vec2>("stick");
	if (stick)
	{
		drawControl2d(hud, position, size, glm::vec2(stick->value.x, -stick->value.y));
	}

	auto pedal = controlInputsComponents.get<float>("pedal");
	if (pedal)
	{
		drawHSlider(hud, position - glm::vec2(0.0f, size * 0.5f + margin), size, pedal->value * 0.5f + 0.5f);
	}

	auto collective = controlInputsComponents.get<float>("collective");
	if (collective)
	{
		drawVSlider(hud, position - glm::vec2(size * 0.5f + margin, 0.0f), size, collective->value);
	}
}

void HudSystem::updatePostDynamics(const sim::System::StepArgs& args)
{
	bool showHud = false;
	if (mEntity && mEnabled)
	{
		osg::Viewport* viewport = mParentCamera->getViewport();
		mHud->setAspectRatio(viewport->width() / viewport->height());

		Node* node = mEntity->getFirstComponent<Node>().get();
		DynamicBodyComponent* body = mEntity->getFirstComponent<DynamicBodyComponent>().get();

		if (node && body)
		{
			showHud = true;
			mHud->clear();

			LatLonAlt lla = toLatLonAlt(GeocentricPosition(node->getPosition())).position;
			Quaternion ltpOrientation = toLtpNed(GeocentricOrientation(node->getOrientation()), toLatLon(lla)).orientation;
			Vector3 euler = math::eulerFromQuat(ltpOrientation);

			mHeadingRibbon->draw(euler.z * math::radToDegD());

			Quaternion uprightYawFrameOrientation = toGeocentric(LtpNedOrientation(glm::angleAxis(euler.z, Vector3(0, 0, 1))), toLatLon(lla)).orientation;
			const Vector3 uprightLocalVel = glm::inverse(uprightYawFrameOrientation) * body->getLinearVelocity();
			const glm::vec2 uprightLocalVel2d(uprightLocalVel.x, uprightLocalVel.y);

			if (glm::length(uprightLocalVel2d) < 10)
			{
				mHoverVelModel->draw(uprightLocalVel2d);
			}

			if (mPitchLadderVisible)
			{
				double verticalFov = mVerticalFovProvider();
				mPitchLadder->setPitchGapHeight(2.0 * pitchLadderAngleIncrement / verticalFov);
				mPitchLadder->draw(euler.y, euler.x);
			}

			mHud->drawText(glm::vec2(-0.4, 0.35), toIntString(glm::length(body->getLinearVelocity()) * metersPerSecToKnots));

			mAltitudeBar->draw(lla.alt);
			mHud->drawText(glm::vec2(0.4, 0.35), toIntString(lla.alt * metersToFeet));

			mRollAngleRibbon->draw(euler.x * math::radToDegD());

			drawVee(*mHud, glm::vec2(0));

			ControlInputsComponent* controlInputsComponents = mEntity->getFirstComponent<ControlInputsComponent>().get();
			if (controlInputsComponents)
			{
				drawControlInputs(*mHud, glm::vec2(-0.8), *controlInputsComponents);
			}
		}
	}

	if (showHud && !mInScene)
	{
		mParentCamera->addChild(mHud);
		mInScene = true;
	}
	else if (!showHud && mInScene)
	{
		mParentCamera->removeChild(mHud);
		mInScene = false;
	}
}

void HudSystem::setEntity(sim::Entity* entity)
{
	if (mEntity != entity)
	{
		if (mEntity)
		{
			mEntity->removeListener(this);
		}

		mEntity = entity;

		if (mEntity)
		{
			mEntity->addListener(this);
		}
	}
}

void HudSystem::onDestroy(Entity* entity)
{
	assert(entity == mEntity);
	setEntity(nullptr);
}
