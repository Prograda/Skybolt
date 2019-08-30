/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SimpleNdm.h"
#include <QLabel>

class DisplayNdm : public SimpleNdm
{
public:
	DisplayNdm(NodeContext* context);

	static QString Name() { return "Display"; }

	QWidget* embeddedWidget() override { return label; }

	void eval(const NodeDataVector& inputs) const override;

private:
	QLabel* label;
};
