/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"
#include "SkyboltQt/Scenario/Registry.h"

#include <SkyboltEngine/Scenario/ScenarioObjectPath.h>
#include <SkyboltSim/SimMath.h>

#include <functional>
#include <optional>
#include <vector>
#include <string>
#include <typeindex>

#include <QIcon>

class ScenarioObject
{
public:
	virtual ~ScenarioObject() = default;

	virtual std::string getUniqueName() const = 0;
	virtual std::string getDisplayName() const = 0;
	virtual const QIcon& getIcon() const = 0;

	virtual const skybolt::ScenarioObjectPath& getDirectory() const
	{
		static skybolt::ScenarioObjectPath path;
		return path;
	}
	virtual void setDirectory(const skybolt::ScenarioObjectPath& path) {}

	virtual std::optional<skybolt::sim::Vector3> getWorldPosition() const { return std::nullopt; } //!< Get position of the object in the world
	virtual void setWorldPosition(const skybolt::sim::Vector3& position) {}

	//! Intersects ray against the object. By default the object is assumed to be an icon with a world position, and fixed size in screen space.
	virtual std::optional<skybolt::sim::Vector3> intersectRay(const skybolt::sim::Vector3& origin, const skybolt::sim::Vector3& dir, const glm::dmat4& viewProjTransform) const;
};

template <typename T>
class ScenarioObjectT : public ScenarioObject
{
public:
	ScenarioObjectT(const std::string& name, const QIcon& icon, const T& data) :
		name(name), icon(icon), data(data) {}

	~ScenarioObjectT() override = default;

	std::string getUniqueName() const override { return name; }
	std::string getDisplayName() const override { return name; }
	const QIcon& getIcon() const override { return icon; }

	const skybolt::ScenarioObjectPath& getDirectory() const override { return mDirectory; }
	void setDirectory(const skybolt::ScenarioObjectPath& path) override { mDirectory = path; }

	virtual const T& getData() const { return data; }

	const std::string name;
	const QIcon icon;
	const T data;

protected:
	skybolt::ScenarioObjectPath mDirectory;
};

using ScenarioObjectRegistry = Registry<ScenarioObject>;
using ScenarioObjectRegistryPtr = std::shared_ptr<ScenarioObjectRegistry>;

struct ScenarioObjectType
{
	virtual ~ScenarioObjectType() = default;

	std::string name; //!< Name of the type
	std::vector<std::string> templateNames; //!< Names of each templates of each type. For example, if the type is "entity", the templates would be things like "cargo ship" etc. Can be empty.
	std::function<void(const std::string& instanceName, const std::string& templateName)> objectCreator; //!< Creates object. Null if objects of this type cannot be created by user.
	std::function<bool(const ScenarioObject&)> isObjectRemovable; //!< Returns true if object can be removed by user. Never null.
	std::function<void(const ScenarioObject&)> objectRemover; //!< Removes object from the scenario. Has no effect if object is not removable. Never null.
	ScenarioObjectRegistryPtr objectRegistry; //!< Registry of instantiated objects of this type. Never null.
};
