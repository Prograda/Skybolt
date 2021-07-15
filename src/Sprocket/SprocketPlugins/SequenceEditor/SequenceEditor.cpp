/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SequenceEditor.h"
#include <Sprocket/Entity/EntityChooserDialogFactory.h>
#include <SkyboltEngine/TimeSource.h>
#include <SkyboltEngine/Sequence/JulianDateSequenceController.h>
#include <SkyboltEngine/Sequence/EntityStateSequenceController.h>
#include <SkyboltSim/Components/NameComponent.h>

#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QTableWidget>

#include <assert.h>

using namespace skybolt;

QString toSequenceTypeName(const std::type_index& type)
{
	static std::map<std::type_index, QString> types = {
		{typeid(JulianDateSequenceController), "Date Time"},
		{typeid(EntityStateSequenceController), "Entity State"}
	};
	auto i = types.find(type);
	if (i != types.end())
	{
		return i->second;
	}
	return "";
}

SequenceEditor::SequenceEditor(const SequenceEditorConfig& config) :
	QWidget(config.parent),
	mController(config.controller),
	mTimeSource(config.timeSource),
	mSequenceRecorder(config.sequenceRecorder),
	mTable(new QTableWidget)
{
	assert(mController);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(new QLabel("Sequence Type: " + toSequenceTypeName(typeid(*mController))));

	if (EntityStateSequenceController* entityStateSequenceController = dynamic_cast<EntityStateSequenceController*>(mController.get()))
	{
		addEntityControls(mainLayout, entityStateSequenceController, config.entityChooserDialogFactory);
	}

	mTable->setColumnCount(2);
	mTable->setHorizontalHeaderLabels({ "Time", "Value" });
	mTable->setSelectionMode(QAbstractItemView::SingleSelection);
	mTable->setSelectionBehavior(QAbstractItemView::SelectRows);

	SequencePtr sequence = mController->getSequence();
	for (size_t i = 0; i < sequence->times.size(); ++i)
	{
		addItem(i);
	}

	connect(mTable, &QTableWidget::doubleClicked, [=](const QModelIndex& index) {
		mTimeSource->setTime(sequence->times[index.row()]);
	});

	mConnections.push_back(sequence->itemAdded.connect([=](size_t index) {
		addItem(index);
		updateSetKeyButton();
	}));

	mConnections.push_back(sequence->itemRemoved.connect([=](size_t index) {
		mTable->removeRow((int)index);
		updateSetKeyButton();
	}));

	mConnections.push_back(sequence->valueChanged.connect([=](size_t index) {
		const SequenceState& value = sequence->getItemAtIndex(index);
		mTable->item((int)index, 1)->setText(QString::fromStdString(value.toString()));
	}));

	mSetKeyButton = new QPushButton();
	
	connect(mSetKeyButton, &QPushButton::pressed, [this]() {
		setKeyAtCurrentTime();
	});

	QPushButton* deleteKeyButton = new QPushButton("Delete Key");
	deleteKeyButton->setEnabled(mTable->currentIndex().isValid());
	
	connect(deleteKeyButton, &QPushButton::pressed, [=]() {
		auto index = mTable->currentIndex();
		if (index.isValid())
		{
			sequence->removeItemAtIndex(index.row());
		}
	});

	connect(mTable->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](const QModelIndex& index, const QModelIndex& prevIndex) {
		deleteKeyButton->setEnabled(index.isValid());
	});

	mConnections.push_back(mTimeSource->timeChanged.connect([this, sequence](double time) {
		updateSetKeyButton();
	}));
	updateSetKeyButton();

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(mSetKeyButton);
	buttonLayout->addWidget(deleteKeyButton);

	mainLayout->addLayout(buttonLayout);
	mainLayout->addWidget(mTable);

	QPushButton* recordButton = new QPushButton("Record");
	recordButton->setCheckable(true);
	recordButton->setChecked(config.isRecording);
	mainLayout->addWidget(recordButton);

	connect(recordButton, &QPushButton::toggled, this, [this] (bool checked) {
		mSequenceRecorder(checked);
	});

	setLayout(mainLayout);
}

void SequenceEditor::addItem(size_t index) {
	auto sequence = mController->getSequence();
	mTable->insertRow((int)index);
	const double& time = sequence->times[index];
	const SequenceState& value = sequence->getItemAtIndex(index);

	QTableWidgetItem* item  = new QTableWidgetItem(QString::number(time));
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	mTable->setItem((int)index, 0, item);

	item = new QTableWidgetItem(QString::fromStdString(value.toString()));
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	mTable->setItem((int)index, 1, item);
}

void SequenceEditor::updateSetKeyButton()
{
	mSetKeyButton->setEnabled(mController->getState() != nullptr);

	auto sequence = mController->getSequence();
	if (sequence->getIndexAtTime(mTimeSource->getTime()))
	{
		mSetKeyButton->setText("Set Key");
	}
	else
	{
		mSetKeyButton->setText("Create Key");
	}
}

void SequenceEditor::addEntityControls(QBoxLayout* mainLayout, skybolt::EntityStateSequenceController* controller, const EntityChooserDialogFactoryPtr& entityChooserFactory)
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(new QLabel("Entity:"));

	QLabel* entityName = new QLabel();
	layout->addWidget(entityName);

	QPushButton* chooseEntityButton = new QPushButton("Choose");
	layout->addWidget(chooseEntityButton);

	connect(chooseEntityButton, &QPushButton::pressed, [=] {
		sim::EntityPtr entity = entityChooserFactory->chooseEntity();
		if (entity)
		{
			controller->setEntity(entity.get());
		}
	});

	auto updateEntityName = [=](const sim::Entity* entity) {
		entityName->setText(entity ? QString::fromStdString(sim::getName(*entity)) : "(None)");
	};

	mConnections.push_back(controller->entityChanged.connect([=] (const sim::Entity* entity) {
		updateEntityName(entity);
		updateSetKeyButton();
	}));
	updateEntityName(controller->getEntity());

	layout->addStretch();

	mainLayout->addLayout(layout);
}

void SequenceEditor::setKeyAtCurrentTime()
{
	auto state = mController->getState();
	auto sequence = mController->getSequence();
	if (state)
	{
		double time = mTimeSource->getTime();
		auto index = sequence->getIndexAtTime(time);
		if (index)
		{
			sequence->setValueAtIndex(*state, *index);
		}
		else
		{
			sequence->addItemAtTime(*state, time);
		}
	}
}