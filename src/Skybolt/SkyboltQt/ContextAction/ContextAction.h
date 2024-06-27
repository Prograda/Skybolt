/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>

template <typename T>
class ContextAction
{
public:
	virtual ~ContextAction() {}

	virtual std::string getName() const = 0;

	virtual bool handles(const T& object) const = 0;

	virtual void execute(T& object) const = 0;
};
