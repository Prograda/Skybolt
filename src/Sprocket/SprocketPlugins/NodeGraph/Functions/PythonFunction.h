/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "FlowFunction.h"

class PythonFunction : public FlowFunction
{
public:
	PythonFunction(const NodeDefPtr& nodeDef);

	void setCode(const std::string& code);
	const std::string& getCode() const { return mCode; }

	NodeDataPtrVector eval(const NodeDataPtrVector& inputs) const override;

private:
	std::shared_ptr<struct _object> mCompiledCode;
	std::string mCode;
};
