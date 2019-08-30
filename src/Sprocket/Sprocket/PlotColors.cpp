/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PlotColors.h"

// Generated with http://tools.medialab.sciences-po.fr/iwanthue
static std::vector<QColor> colors = {
	QColor("#d44e2d"),
	QColor("#6a78e0"),
	QColor("#76bd5c"),
	QColor("#cca63a"),
	QColor("#43343c"),
	QColor("#a4da39"),
	QColor("#bc557e"),
	QColor("#879fc0"),
	QColor("#ae5944"),
	QColor("#cd54b9"),
	QColor("#62c09e"),
	QColor("#8f46d6"),
	QColor("#92925c"),
	QColor("#614a98"),
	QColor("#aca79e")
};

std::vector<QColor> getPlotColors()
{
	return colors;
}