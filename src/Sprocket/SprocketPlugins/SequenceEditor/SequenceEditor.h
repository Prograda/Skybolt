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
	std::function<void(bool)> sequenceRecorder; //! set to true to enable recording
	bool isRecording = false;
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
	void setKeyAtCurrentTime();

private:
	skybolt::StateSequenceControllerPtr mController;
	skybolt::TimeSource* mTimeSource;
	std::function<void(bool)> mSequenceRecorder;
	QPushButton* mSetKeyButton;
	std::vector<boost::signals2::scoped_connection> mConnections;
	QTableWidget* mTable;
	bool mRecording = false;
};