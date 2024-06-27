/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PropertyEditor.h"
#include "SkyboltQt/QtUtil/QtLayoutUtil.h"

#include <QtCore/QDate>
#include <QtCore/QLocale>
#include <QLabel>
#include <QLayout>

#include <assert.h>

using namespace skybolt;

PropertyEditor::PropertyEditor(const PropertyEditorWidgetFactoryMap& factoryMap, QWidget* parent) :
	QWidget(parent),
	mFactoryMap(factoryMap),
	mGridLayout(new QGridLayout(this))
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

	clearLayout(*mGridLayout);

	int r = 0;
	for (const QtPropertyPtr& property : model->getProperties())
	{
		auto label = new QLabel(property->name, this);
		auto labelLayout = new QVBoxLayout();
		labelLayout->setContentsMargins(3, 3, 6, 3);
		labelLayout->addWidget(label);
		mGridLayout->addLayout(labelLayout, r, 0, Qt::AlignTop | Qt::AlignRight);
		QWidget* widget = createEditor(property.get());
		if (widget)
		{
			mGridLayout->addWidget(widget, r, 1);
			mGridLayout->setRowStretch(r, -1);
		}

		++r;
	}
	mGridLayout->setRowStretch(r, 1);
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
	auto i = mFactoryMap.find(property->value.userType());
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
