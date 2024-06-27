/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/SkyboltSimFwd.h"

class QWidget;

class EntityChooserDialogFactory
{
public:
	EntityChooserDialogFactory(const skybolt::sim::World* world);
	skybolt::sim::EntityPtr chooseEntity() const;

private:
	const skybolt::sim::World* mWorld;
};