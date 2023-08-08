#pragma once

#include <SkyboltSim/SkyboltSimFwd.h>

#include <QWidget>
#include <string>
#include <boost/signals2.hpp>

class EntityListModel;
class QComboBox;

class CameraControllerWidget : public QWidget
{
public:
	CameraControllerWidget(skybolt::sim::World* world, QWidget* parent = nullptr);

	void setCamera(const skybolt::sim::EntityPtr& camera);

private:
	void updateTargetFilterForControllerName(const std::string& name);

private:
	skybolt::sim::World* mWorld;
	skybolt::sim::EntityPtr mCamera;
	QComboBox* mCameraModeCombo;
	QComboBox* mCameraTargetCombo;
	EntityListModel* mTargetListModel;
	std::vector<boost::signals2::scoped_connection> mControllerConnections;
};