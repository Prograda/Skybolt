/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <QWidget>

#include <Sprocket/SprocketFwd.h>
#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltEngine/Sequence/SequenceController.h>

class QPushButton;
class QTableWidget;
class QBoxLayout;

struct SequenceEditorConfig
{
	skybolt::StateSequenceControllerPtr controller;
	skybolt::TimeSource* timeSource;
	EntityChooserDialogFactoryPtr entityChooserDialogFactory;
	QWidget* parent = nullptr;
};

class SequenceEditor : public QWidget
{
public:
	SequenceEditor(const SequenceEditorConfig& config);

private:
	void addItem(size_t index);
	void updateSetKeyButton();
	void addEntityControls(QBoxLayout* mainLayout, skybolt::EntityStateSequenceController* controller, const EntityChooserDialogFactoryPtr& entityChooserFactory);

private:
	skybolt::StateSequenceControllerPtr mController;
	skybolt::TimeSource* mTimeSource;
	QPushButton* mSetKeyButton;
	std::vector<boost::signals2::scoped_connection> mConnections;
	QTableWidget* mTable;
};