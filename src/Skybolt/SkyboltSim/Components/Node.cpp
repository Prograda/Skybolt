/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "Node.h"
#include "PropertyMetadata.h"
#include "SkyboltSim/Entity.h"

namespace skybolt::sim {

SKYBOLT_REFLECT_BEGIN(Node)
{
	registry.type<Node>("Node")
		.superType<Component>()
		.property("position", &Node::getPosition, &Node::setPosition, {{PropertyMetadataNames::attributeType, AttributeType::PositionInWorld}})
		.property("orientation", &Node::getOrientation, &Node::setOrientation, {{PropertyMetadataNames::attributeType, AttributeType::Orientation}});
}
SKYBOLT_REFLECT_END

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

} // namespace skybolt::sim