/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SprocketFwd.h"
#include "TableRecord.h"
#include <QWidget>
#include <memory>
#include <typeindex>

class QGridLayout;

struct QtProperty : public QObject
{
	Q_OBJECT
public:
	QString name;
	bool enabled = true;

	void setEnabled(bool e) {
		if (enabled != e)
		{
			enabled = e;
			emit enabledChanged(enabled);
		}
	}

signals:
	void valueChanged();
	void enabledChanged(bool enabled);
};

struct VariantProperty : public QtProperty
{
	void setValue(const QVariant& v)
	{
		if (value != v)
		{
			value = v;
			emit valueChanged();
		}
	}

	QVariant value;
};

struct EnumProperty : public QtProperty
{
	void setValue(int v)
	{
		if (value != v)
		{
			value = v;
			emit valueChanged();
		}
	}

	int value;
	QStringList values;
};

struct TableProperty : public QtProperty
{
	Q_OBJECT
public:
	QStringList fieldNames;
	std::vector<TableRecord> records;

signals:
	void recordAdded(int index, const TableRecord& record);
	void recordMoved(int oldIndex, int newIndex);
	void recordRemoved(int index);
};

typedef std::shared_ptr<QtProperty> QtPropertyPtr;
typedef std::shared_ptr<VariantProperty> VariantPropertyPtr;
typedef std::shared_ptr<EnumProperty> EnumPropertyPtr;
typedef std::shared_ptr<TableProperty> TablePropertyPtr;

class PropertiesModel : public QObject
{
	Q_OBJECT
public:
	PropertiesModel();
	PropertiesModel(const std::vector<QtPropertyPtr>& properties);
	~PropertiesModel() {}

	virtual std::vector<QtPropertyPtr> getProperties() const { return mProperties; }

	static VariantPropertyPtr createVariantProperty(const QString& name, const QVariant& value);
	static EnumPropertyPtr createEnumProperty(const QString& name, const QStringList& values, int value = 0);
	static TablePropertyPtr createTableProperty(const QString& name, const QStringList& fieldNames, const std::vector<TableRecord>& records);

signals:
	void modelReset(PropertiesModel*);

protected:
	std::vector<QtPropertyPtr> mProperties;
};

using PropertyEditorWidgetFactory = std::function<QWidget*(QtProperty& property)>;
using PropertyEditorWidgetFactoryMap = std::map<std::type_index, PropertyEditorWidgetFactory>;

class PropertyEditor : public QWidget
{
	Q_OBJECT
public:
	PropertyEditor(const PropertyEditorWidgetFactoryMap& factoryMap, QWidget* parent = nullptr);

	void setModel(const PropertiesModelPtr& model);

private slots:
	void modelReset(PropertiesModel* model);

private:
	QWidget* createEditorInEnabledState(QtProperty& property);
	QWidget* createEditor(QtProperty& property);

private:
	PropertiesModelPtr mModel;
	QGridLayout* mGridLayout;
	PropertyEditorWidgetFactoryMap mFactoryMap;
};
