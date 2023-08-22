/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QMouseEvent>

class ViewportMouseEventHandler
{
public:
	virtual ~ViewportMouseEventHandler() = default;
	
	//! @return true if event was handled
	virtual bool mousePressed(const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers) { return false; }
	virtual bool mouseReleased(const QPointF& position, Qt::MouseButton button) { return false; }
	virtual bool mouseMoved(const QPointF& position, Qt::MouseButtons buttons) { return false; }
};