/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Node.h"
#include "SkyboltSim/Entity.h"

using namespace skybolt::sim;

Node::Node(const Vector3 &localPosition, const Quaternion &localOrientation) :
	mPosition(localPosition),
	mOrientation(localOrientation)
{
}

Node::~Node()
{
}

void Node::setPosition(const Vector3 &position)
{
	mPosition = position;
}

void Node::setOrientation(const Quaternion &orientation)
{
	mOrientation = orientation;
}
