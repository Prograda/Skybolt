/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PropertyEditor.h"
#include "Widgets/CollapsiblePanelWidget.h"
#include "SkyboltQt/QtUtil/QtLayoutUtil.h"
#include <SkyboltCommon/MapUtility.h>

#include <QtCore/QDate>
#include <QtCore/QLocale>
#include <QLabel>
#include <QLayout>

#include <assert.h>

using namespace skybolt;

PropertyEditor::PropertyEditor(const PropertyEditorWidgetFactoryMap& factoryMap, QWidget* parent) :
	QWidget(parent),
	mFactoryMap(factoryMap),
	mLayout(new QVBoxLayout(this))
{
}

void PropertyEditor::setModel(const PropertiesModelPtr& model)
{
	if (mModel)
		disconnect(mModel.get(), SIGNAL(modelReset(PropertiesModel*)), this, SLOT(modelReset(PropertiesModel*)));

	mModel = model;

	if (model)
	{
		connect(model.get(), SIGNAL(modelReset(PropertiesModel*)), this, SLOT(modelReset(PropertiesModel*)));
		modelReset(model.get());
	}
	else
	{
		clearLayout(*layout());
	}
}

void PropertyEditor::modelReset(PropertiesModel* model)
{
	assert(mModel.get() == model);

	// Remove existing property widgets
	clearLayout(*mLayout);

	// Add property widgets
	for (const auto& [sectionName, properties] : model->getProperties())
	{
		QGridLayout* gridLayout;

		if (sectionName.empty())
		{
			auto widget = new QWidget();
			mLayout->addWidget(widget);
			gridLayout = new QGridLayout(widget);
		}
		else
		{
			auto section = new CollapsiblePanelWidget(QString::fromStdString(sectionName), this);
			section->setExpanded(skybolt::findOptional(mDefaultSectionExpandedState, sectionName).value_or(true));
			connect(section, &CollapsiblePanelWidget::expandedStateChanged, this, [this, sectionName = sectionName] (bool expanded) {
				mDefaultSectionExpandedState[sectionName] = expanded;
			});

			mLayout->addWidget(section);
			gridLayout = new QGridLayout(section->getContentWidget());
		}
		gridLayout->setColumnMinimumWidth(0, 100);
		gridLayout->setColumnStretch(1, 1);

		int rowCount = 0;
		for (const QtPropertyPtr& property : properties)
		{
			auto label = new QLabel(property->name, this);
			auto labelLayout = new QVBoxLayout();
			labelLayout->setContentsMargins(3, 3, 6, 3);
			labelLayout->addWidget(label);
			gridLayout->addLayout(labelLayout, rowCount, 0, Qt::AlignTop | Qt::AlignRight);
			QWidget* widget = createEditor(property.get());
			if (widget)
			{
				gridLayout->addWidget(widget, rowCount, 1);
				++rowCount;
			}
		}
	}
	mLayout->addStretch(1);
}

static void setEditable(QWidget& widget, bool editable)
{
	if (QVariant v = widget.property("readOnly"); !v.isNull())
	{
		widget.setProperty("readOnly", !editable);
	}
	else
	{
		widget.setEnabled(editable);
	}
}

QWidget* PropertyEditor::createEditor(QtProperty* property)
{
	auto i = mFactoryMap.find(property->value().userType());
	if (i != mFactoryMap.end())
	{
		if (QWidget* widget = i->second(property, this); widget)
		{
			setEditable(*widget, property->enabled);

			connect(property, &QtProperty::enabledChanged, widget, [=](bool enabled) {
				widget->setEnabled(enabled);
			});
			return widget;
		}
	}
	return nullptr;
}
