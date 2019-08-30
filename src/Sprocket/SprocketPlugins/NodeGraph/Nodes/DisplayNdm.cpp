/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "DisplayNdm.h"

DisplayNdm::DisplayNdm(NodeContext* context) :
	label(new QLabel())
{
	NodeDefPtr def = std::make_shared<NodeDef>();
	def->name = Name();
	def->inputs = { { DoubleNodeData::typeId(), "value" } };
	setNodeDef(def);

	label->setMargin(3);
	label->setMinimumWidth(50);
}

void DisplayNdm::eval(const NodeDataVector& inputs) const
{
	label->setText(QString::number(getDataOrDefault<DoubleNodeData>(inputs[0].get())));
	label->adjustSize();
}
