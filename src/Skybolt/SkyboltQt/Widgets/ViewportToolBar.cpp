/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ViewportToolBar.h"
#include "SkyboltQt/Entity/EntityListModel.h"
#include "SkyboltQt/Icon/SkyboltIcons.h"
#include "SkyboltQt/Property/PropertyEditor.h"
#include "SkyboltQt/QtUtil/QtDialogUtil.h"
#include "SkyboltQt/QtUtil/QtTimerUtil.h"
#include "SkyboltQt/Viewport/ViewportPropertiesModel.h"
#include "SkyboltQt/Widgets/CameraControllerWidget.h"
#include "SkyboltQt/Widgets/ViewportWidget.h"

#include <SkyboltEngine/EngineRoot.h>
#include <SkyboltSim/Components/CameraComponent.h>
#include <SkyboltSim/Components/NameComponent.h>
#include <SkyboltSim/World.h>
#include <SkyboltVis/Window/CaptureScreenshot.h>

#include <QComboBox>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QToolButton>

using namespace skybolt;

static sim::Entity* getSelectedCamera(const sim::World& world, const QComboBox& comboBox)
{
	return world.findObjectByName(comboBox.currentText().toStdString()).get();
}

ViewportToolBar::ViewportToolBar(ViewportToolBarConfig config) :
	QToolBar(config.parent),
	mFilterMenu(new QMenu(this))
{
	sim::World* world = &config.engineRoot->scenario->world;

	{
		auto cameraListModel = new EntityListModel(world, [](const sim::Entity& entity) {
			return entity.getFirstComponent<sim::CameraComponent>() != nullptr;
			});

		auto proxyModel = new QSortFilterProxyModel(this);
		proxyModel->sort(0);
		proxyModel->setSourceModel(cameraListModel);

		mCameraCombo = new QComboBox(this);
		mCameraCombo->setToolTip("Camera");
		mCameraCombo->setModel(proxyModel);
		addWidget(mCameraCombo);

		mCameraControllerWidget = new CameraControllerWidget(world, this);
		addWidget(mCameraControllerWidget);

		auto updateViewportSelectedCamera = [this, world, viewport = config.viewport] {
			if (viewport)
			{
				sim::Entity* camera = getSelectedCamera(*world, *mCameraCombo);
				viewport->setCamera(camera);
			}
		};
		updateViewportSelectedCamera();

		connect(mCameraCombo, &QComboBox::currentTextChanged, this, [updateViewportSelectedCamera = std::move(updateViewportSelectedCamera)](const QString& text) {
			updateViewportSelectedCamera();
			});
	}

	addAction(getSkyboltIcon(SkyboltIcon::Screenshot), "Capture Image",
		[this, scenarioFilenameGetter = std::move(config.scenarioFilenameGetter), viewport = config.viewport] {
			if (viewport)
			{
				viewport->captureImage(scenarioFilenameGetter());
			};
		});

	{
		QToolButton* toolButton = new QToolButton(this);
		toolButton->setText("Filter");
		toolButton->setToolTip("Filter");
		toolButton->setIcon(getSkyboltIcon(SkyboltIcon::Filter));
		toolButton->setMenu(mFilterMenu);
		toolButton->setPopupMode(QToolButton::InstantPopup);
		addWidget(toolButton);
	}

	addAction(getSkyboltIcon(SkyboltIcon::Settings), "Settings",
		[this, viewport = config.viewport, scene = config.engineRoot->scene.get()] {
		if (viewport && viewport->getCamera())
		{
			PropertyEditor* editor = new PropertyEditor();
			editor->setModel(std::make_shared<ViewportPropertiesModel>(scene, viewport->getCamera()->getFirstComponent<sim::CameraComponent>().get()));
			QDialog* dialog = createDialogNonModal(editor, "Settings", this);
			dialog->exec();
		}
		});

	createAndStartIntervalTimer(/* intervalMilliseconds */ 100, this, [this, viewport = config.viewport] {
		if (viewport)
		{
			sim::Entity* camera = viewport->getCamera();
			if (mCameraControllerWidget->getCamera() != camera)
			{
				mCameraControllerWidget->setCamera(camera);
				mCameraCombo->setCurrentText(camera ? QString::fromStdString(getName(*camera)) : "");
			}
			mCameraControllerWidget->update();
		}
		});
}