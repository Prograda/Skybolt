/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "RenderOperation.h"

#include <optional>
#include <map>
#include <string>

namespace skybolt {
namespace vis {

class RenderOperationPipeline
{
public:
	RenderOperationPipeline();

	//! @param priority is the relative priority in which to perform the operation. Lower numbers are performed first.
	void addOperation(const osg::ref_ptr<RenderOperation>& operation, int priority);
	void removeOperation(const osg::ref_ptr<RenderOperation>& operation);

	using Priority = int;
	using RenderOperationsOrder = std::multimap<Priority, osg::ref_ptr<RenderOperation>>;

	const RenderOperationsOrder& getOperations() const { return mOperations; }

	void updatePreRender();

	const osg::ref_ptr<osg::Group>& getRootNode() const { return mRootNode; }

private:
	void updateOsgGroup();

private:
	osg::ref_ptr<osg::Group> mRootNode;
	RenderOperationsOrder mOperations;
};

} // namespace vis
} // namespace skybolt
