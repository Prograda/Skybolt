/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/SprocketFwd.h"
#include "Sprocket/Registry.h"
#include <functional>
#include <vector>
#include <string>
#include <typeindex>

#include <QIcon>

class ScenarioObject
{
public:
	virtual ~ScenarioObject() = default;

	virtual std::string getName() const = 0;
	virtual const QIcon& getIcon() const = 0;
	virtual ScenarioObjectPtr getParent() const { return nullptr; }
};

template <typename T>
class ScenarioObjectT : public ScenarioObject
{
public:
	ScenarioObjectT(const std::string& name, const QIcon& icon, const T& data) :
		name(name), icon(icon), data(data) {}

	~ScenarioObjectT() override = default;

	std::string getName() const override { return name; }
	const QIcon& getIcon() const override { return icon; }

	virtual const T& getData() const { return data; }

	const std::string name;
	const QIcon icon;
	const T data;
};

using ScenarioObjectRegistry = Registry<ScenarioObject>;
using ScenarioObjectRegistryPtr = std::shared_ptr<ScenarioObjectRegistry>;

struct ScenarioObjectType
{
	virtual ~ScenarioObjectType() = default;

	std::string name; //!< Name of the type
	std::string category; //!< Name of the category which objects of this type should be grouped under in GUI. If empty, objects will treated as 'top level' rather than being grouped.
	std::vector<std::string> templateNames; //!< Names of each templates of each type. For example, if the type is "entity", the templates would be things like "cargo ship" etc. Can be empty.
	std::function<void(const std::string& instanceName, const std::string& templateName)> objectCreator; //!< Creates object. Null if objects of this type cannot be created by user.
	std::function<bool(const ScenarioObject&)> isObjectRemovable; //!< Returns true if object can be removed by user. Never null.
	std::function<void(const ScenarioObject&)> objectRemover; //!< Removes object from the scenario. Has no effect if object is not removable. Never null.
	ScenarioObjectRegistryPtr objectRegistry; //!< Registry of instantiated objects of this type. Never null.
};

using ScenarioObjectTypeMap = std::map<std::type_index, ScenarioObjectTypePtr>; //!< Type index is of the class derived from ScenarioObject which this type creates