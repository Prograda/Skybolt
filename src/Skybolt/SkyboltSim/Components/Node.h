/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include "SkyboltSim/Spatial/Positionable.h"
#include <SkyboltCommon/Math/MathUtility.h>

namespace skybolt {
namespace sim {

class Node : public Positionable, public Component
{
public:
	Node(const Vector3 &localPosition = math::dvec3Zero(), const Quaternion &localOrientation = math::dquatIdentity());

	~Node() override;

	void setPosition(const Vector3 &position);
	void setOrientation(const Quaternion &orientation);

	Vector3 getPosition() const override {return mPosition;}
	Quaternion getOrientation() const override {return mOrientation;}

private:
	Vector3 mPosition;
	Quaternion mOrientation;
};

SKYBOLT_REFLECT_EXTERN(Node)

} // namespace sim
} // namespace skybolt