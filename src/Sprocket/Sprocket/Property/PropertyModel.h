/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"
#include "Sprocket/TableRecord.h"
#include <SkyboltCommon/Updatable.h>

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

class PropertiesModel : public QObject, public skybolt::Updatable
{
	Q_OBJECT
public:
	PropertiesModel();
	PropertiesModel(const std::vector<QtPropertyPtr>& properties);
	~PropertiesModel() {}

	void update() override;

	virtual std::vector<QtPropertyPtr> getProperties() const { return mProperties; }

	using QtPropertyUpdater = std::function<void(QtProperty&)> ;
	using QtPropertyApplier = std::function<void(const QtProperty&)>;

	//! @param updater is regularly called update the value of QtProperty from an external model
	//! @param applier is called when a QtProperty value should be applied to an external model (e.g. if the user pressent 'Enter' key in a text box
	void addProperty(const QtPropertyPtr& property, QtPropertyUpdater updater = nullptr, QtPropertyApplier applier = nullptr);

	static VariantPropertyPtr createVariantProperty(const QString& name, const QVariant& value);
	static EnumPropertyPtr createEnumProperty(const QString& name, const QStringList& values, int value = 0);
	static TablePropertyPtr createTableProperty(const QString& name, const QStringList& fieldNames, const std::vector<TableRecord>& records);

signals:
	void modelReset(PropertiesModel*);

protected:
	std::vector<QtPropertyPtr> mProperties;
	std::map<QtPropertyPtr, QtPropertyUpdater> mPropertyUpdaters;
	bool mCurrentlyUpdating = false;
};
