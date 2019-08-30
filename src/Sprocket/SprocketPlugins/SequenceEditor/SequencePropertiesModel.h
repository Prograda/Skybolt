/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <Sprocket/PropertyEditor.h>
#include <SkyboltEngine/Sequence/SequenceController.h>

struct SequenceProperty : public QtProperty
{
	SequenceProperty(skybolt::StateSequenceControllerPtr sequence) : sequence(sequence) {}
	skybolt::StateSequenceControllerPtr sequence;
};

class SequencePropertiesModel : public PropertiesModel
{
	Q_OBJECT
public:
	SequencePropertiesModel(const skybolt::StateSequenceControllerPtr& sequence);

private:
	skybolt::StateSequenceControllerPtr mSequence;
};
