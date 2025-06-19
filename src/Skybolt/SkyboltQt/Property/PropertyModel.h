/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include <SkyboltCommon/Updatable.h>

#include <QObject>
#include <QVariant>
#include <memory>

struct QtProperty : public QObject
{
	Q_OBJECT
public:
	QString name;
	bool enabled = true;

	void setEnabled(bool e)
	{
		if (enabled != e)
		{
			enabled = e;
			emit enabledChanged(enabled);
		}
	}

	void setValue(const QVariant& v)
	{
		if (value != v)
		{
			value = v;
			emit valueChanged();
		}
	}

	QVariant value;

signals:
	void valueChanged();
	void enabledChanged(bool enabled);
};

QtPropertyPtr createQtProperty(const QString& name, const QVariant& value);

class PropertiesModel : public QObject, public skybolt::Updatable
{
	Q_OBJECT
public:
	using Properties = std::vector<QtPropertyPtr>;
	using SectionProperties = std::map<std::string, Properties>;

	PropertiesModel();
	PropertiesModel(const SectionProperties& properties);
	~PropertiesModel() {}

	void update() override;

	virtual SectionProperties getProperties() const { return mProperties; }

	using QtPropertyUpdater = std::function<void(QtProperty&)> ;
	using QtPropertyApplier = std::function<void(const QtProperty&)>;

	static const std::string& getDefaultSectionName()
	{
		static std::string s;
		return s;
	}

	//! @param updater is regularly called update the value of QtProperty from an external model
	//! @param applier is called when a QtProperty value should be applied to an external model (e.g. if the user pressent 'Enter' key in a text box
	void addProperty(const QtPropertyPtr& property, QtPropertyUpdater updater = nullptr, QtPropertyApplier applier = nullptr, const std::string& sectionName = getDefaultSectionName());

signals:
	void modelReset(PropertiesModel*);

protected:
	SectionProperties mProperties;
	std::map<QtPropertyPtr, QtPropertyUpdater> mPropertyUpdaters;
	bool mCurrentlyUpdating = false;
};
