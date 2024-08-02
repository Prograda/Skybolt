/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QLayout>

//! Deletes all items in a QLayout
void clearLayout(QLayout& layout);

void addWidgetWithLabel(QGridLayout& layout, QWidget* widget, const QString& label);

QBoxLayout* createBoxLayoutWithWidgets(const std::vector<QWidget*>& widgets, QWidget* parent = nullptr, QBoxLayout::Direction direction = QBoxLayout::Direction::LeftToRight);
