/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/DefaultRootNode.h"
#include "SkyboltVis/OsgBox2.h"
#include <px_sched/px_sched.h>
#include <osg/Image>
#include <atomic>
#include <functional>

namespace skybolt {
namespace vis {

typedef std::function<osg::Vec2f(const osg::Vec2f&)> Vec2Transform;

class PagedForest : public DefaultRootNode
{
public:
	PagedForest(px_sched::Scheduler& scheduler, const osg::ref_ptr<osg::Image>& attributeMap, const ShaderPrograms* programs,
		        const ElevationProviderPtr& elevationProvider, const Box2f& bounds, const osg::Vec2f& maxPageSize,
				Vec2Transform converter, float visRangeWorldUnits);

	~PagedForest();

	void update(const osg::Vec2f& cameraPosition);

	void updatePreRender(const RenderContext& context) override;

private:
	void asyncCreatePage(const osg::Vec2i& pageId);
	void processLoadingPageQueue(const osg::Vec2i& minPageId, const osg::Vec2i& maxPageId);
	void cancelAllLoadingTiles();

	osg::Vec2i getPageId(const osg::Vec2f& position) const;

private:
	std::unique_ptr<class PageGeneratorTask> mPageGeneratorTask;
	const Box2f mBounds;
	osg::Vec2f mPageSize;
	const osg::Vec2f mVisibilityRange;

	typedef std::map<osg::Vec2i, osg::Group*> PagesMap;
	PagesMap mPages; //!< osg::Group* is be null when loading
	osg::Vec2i mPageCount;

	osg::Uniform* mModelMatrixUniform;

	struct LoadingPage
	{
		osg::Vec2i pageId;
		osg::ref_ptr<osg::Group> group; // TODO: Ensure this is consistent across multiple threads
		std::atomic<bool> cancel = false; //!< Signals the loading task to cancel
	};
	typedef std::shared_ptr<LoadingPage> LoadingPagePtr;

	px_sched::Scheduler& mScheduler;
	std::vector<LoadingPagePtr> mLoadingPageQueue;
	px_sched::Sync mLoadingPageSync;
};

} // namespace vis
} // namespace skybolt
