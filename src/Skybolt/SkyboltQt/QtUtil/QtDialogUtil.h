/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QDialog>
#include <QString>
#include <memory>

std::shared_ptr<QDialog> createDialogModal(QWidget* content, const QString& title);

QDialog* createDialogNonModal(QWidget* content, const QString& title, QWidget* parent = nullptr);