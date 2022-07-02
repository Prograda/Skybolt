/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "PagedForest.h"
#include "BillboardForest.h"
#include "ForestGenerator.h"

#include "SkyboltVis/GeoImageHelpers.h"
#include "SkyboltVis/LlaToNedConverter.h"
#include "SkyboltVis/OsgMathHelpers.h"
#include "SkyboltVis/Scene.h"
#include "SkyboltVis/Shader/ShaderProgramRegistry.h"
#include <SkyboltCommon/Math/MathUtility.h>
#include <osg/Geode>
#include <atomic>
#include <deque>
#include <mutex>
#include <thread>

using namespace skybolt;

namespace skybolt {
namespace vis {

class PageGeneratorTask
{
public:
	PageGeneratorTask(const osg::ref_ptr<osg::Image>& attributeMap, const ShaderPrograms* programs,
		const ElevationProviderPtr& elevationProvider, const Box2f& bounds, Vec2Transform converter, float visRangeWorldUnits, const osg::Vec2f pageSize) :
		mConverter(converter),
		mAttributeMap(attributeMap),
		mPrograms(programs),
		mElevationProvider(elevationProvider),
		mBounds(bounds),
		mVisRangeWorldUnits(visRangeWorldUnits),
		mPageSize(pageSize)
	{
	}

	//! Can be called on multiple threads concurrently
	osg::Group* run(const osg::Vec2i& pageId) const
	{
		ForestGenerator::AttributeImage image;
		image.data = (char*)mAttributeMap->getDataPointer();
		image.width = mAttributeMap->s();
		image.height = mAttributeMap->t();

		Box2f worldBounds;
		worldBounds.minimum = mBounds.minimum + osg::Vec2f(pageId.x() * mPageSize.x(), pageId.y() * mPageSize.y());
		worldBounds.maximum = worldBounds.minimum + mPageSize;
		worldBounds.maximum = osg::Vec2f(std::min(worldBounds.maximum.x(), mBounds.maximum.x()), std::min(worldBounds.maximum.y(), mBounds.maximum.y()));

		Box2f imageBounds = getSubImageBounds(mBounds, worldBounds, image.width, image.height);

		ForestGenerator::Attribute attribute;
		const float treesPerSquareMeter = 0.01f;
		const float radiusOfEarth = 6371000.0f;
		attribute.density = treesPerSquareMeter * radiusOfEarth * radiusOfEarth; // trees per sterradian
		image.attributes[2] = attribute;

		ForestGenerator generator;
		std::vector<BillboardForest::Tree> trees = generator.generate(*mElevationProvider, image, worldBounds, imageBounds);

		// Convert tree positions
		for (BillboardForest::Tree& tree : trees)
		{
			osg::Vec2f pos = mConverter(osg::Vec2f(tree.position.x(), tree.position.y()));
			tree.position.x() = pos.x();
			tree.position.y() = pos.y();
		}

		osg::Group* group = new osg::Group;
		BillboardForest::addGeodes(*group, trees, mPrograms->getRequiredProgram("treeSideBillboard"), mPrograms->getRequiredProgram("treeTopBillboard"), mVisRangeWorldUnits, worldBounds.size());
		return group;
	}

private:
	const Vec2Transform mConverter;
	const osg::ref_ptr<osg::Image> mAttributeMap;
	const ShaderPrograms* mPrograms;
	const ElevationProviderPtr mElevationProvider;
	const Box2f mBounds;
	const float mVisRangeWorldUnits;
	const osg::Vec2f mPageSize;
};

PagedForest::PagedForest(px_sched::Scheduler& scheduler, const osg::ref_ptr<osg::Image>& attributeMap, const ShaderPrograms* programs,
						 const ElevationProviderPtr& elevationProvider, const Box2f& bounds, const osg::Vec2f& maxPageSize,
						 Vec2Transform converter, float visRangeWorldUnits) :
	mScheduler(scheduler),
	mBounds(bounds),
	mVisibilityRange(maxPageSize.x(), maxPageSize.y())
{
	osg::Vec2f pageCountF = math::componentWiseDivide(bounds.size(), maxPageSize);
	mPageCount.x() = (int)std::ceil(pageCountF.x());
	mPageCount.y() = (int)std::ceil(pageCountF.y());
	assert(mPageCount.x() > 0);
	assert(mPageCount.y() > 0);

	mPageSize = math::componentWiseDivide(bounds.size(), osg::Vec2f(mPageCount.x(), mPageCount.y()));

	mModelMatrixUniform = new osg::Uniform("modelMatrix", osg::Matrixf());
	mTransform->getOrCreateStateSet()->addUniform(mModelMatrixUniform);

	mPageGeneratorTask.reset(new PageGeneratorTask(attributeMap, programs, elevationProvider, bounds, converter, visRangeWorldUnits, mPageSize));
}

PagedForest::~PagedForest()
{
	cancelAllLoadingTiles();
	mScheduler.waitFor(mLoadingPageSync);
}

bool inValidRange(const osg::Vec2i& pageId, const osg::Vec2i& minPageId, const osg::Vec2i& maxPageId)
{
	return pageId.x() >= minPageId.x() && pageId.x() <= maxPageId.x() && pageId.y() >= minPageId.y() && pageId.y() <= maxPageId.y();
}

void PagedForest::update(const osg::Vec2f& cameraPosition)
{
	osg::Vec2i minPageId = getPageId(osg::Vec2f(cameraPosition.x() - mVisibilityRange.x(), cameraPosition.y() - mVisibilityRange.y()));
	osg::Vec2i maxPageId = getPageId(osg::Vec2f(cameraPosition.x() + mVisibilityRange.x(), cameraPosition.y() + mVisibilityRange.y()));

	if (maxPageId.x() < 0 || maxPageId.y() < 0 ||
		minPageId.x() >= mPageCount.x() || minPageId.y() >= mPageCount.y())
	{
		mTransform->removeChildren(0, mTransform->getNumChildren());
		mPages.clear();
		cancelAllLoadingTiles();
		return;
	}

	minPageId.x() = math::clamp(minPageId.x(), 0, mPageCount.x() - 1);
	minPageId.y() = math::clamp(minPageId.y(), 0, mPageCount.y() - 1);

	maxPageId.x() = math::clamp(maxPageId.x(), 0, mPageCount.x() - 1);
	maxPageId.y() = math::clamp(maxPageId.y(), 0, mPageCount.y() - 1);

	PagesMap newPages;

	// Transfer nearby existing pages over to new pages
	for (PagesMap::iterator i = mPages.begin(); i != mPages.end(); ++i)
	{
		const osg::Vec2i& pageId = i->first;
		osg::ref_ptr<osg::Group> group = i->second;
		// keep existing page if it's within new range
		if (inValidRange(pageId, minPageId, maxPageId))
		{
			newPages[i->first] = group;
		}
		else if (i->second)
		{
			mTransform->removeChild(group);
		}
	}

	std::swap(mPages, newPages);

	// Create nearby pages if they don't exist yet
	for (int y = minPageId.y(); y <= maxPageId.y(); ++y)
	{
		for (int x = minPageId.x(); x <= maxPageId.x(); ++x)
		{
			osg::Vec2i pageId(x,y);
			auto r = mPages.insert(PagesMap::value_type(pageId, nullptr));
			if (r.second) // if inserted
			{
				asyncCreatePage(pageId);
			}
		}
	}

	processLoadingPageQueue(minPageId, maxPageId);
}

void PagedForest::asyncCreatePage(const osg::Vec2i& pageId)
{
	LoadingPagePtr page(new LoadingPage);
	page->pageId = pageId;
	mLoadingPageQueue.push_back(page);

	mScheduler.run([=]() {
		if (!page->cancel)
		{
			page->group = mPageGeneratorTask->run(page->pageId);
		}
	}, &mLoadingPageSync);
}

void PagedForest::processLoadingPageQueue(const osg::Vec2i& minPageId, const osg::Vec2i& maxPageId)
{
	for (size_t i = 0; i < mLoadingPageQueue.size();)
	{
		LoadingPage& page = *mLoadingPageQueue[i];
		bool erase = !inValidRange(page.pageId, minPageId, maxPageId);
		if (erase)
		{
			page.cancel = true;
		}
		else
		{
			osg::ref_ptr<osg::Group> group = page.group;
			if (group)
			{
				mTransform->addChild(group);
				mPages[page.pageId] = group;
				erase = true;
			}
		}

		if (erase)
		{
			mLoadingPageQueue.erase(mLoadingPageQueue.begin() + i);
		}
		else
		{
			++i;
		}
	}
}

void PagedForest::cancelAllLoadingTiles()
{
	for (const LoadingPagePtr& page : mLoadingPageQueue)
	{
		page->cancel = true;
	}
	mLoadingPageQueue.clear();
}

osg::Vec2i PagedForest::getPageId(const osg::Vec2f& position) const
{
	return osg::Vec2i(int((position.x() - mBounds.minimum.x()) / mPageSize.x()),
					  int((position.y() - mBounds.minimum.y()) / mPageSize.y()));
}

void PagedForest::updatePreRender(const CameraRenderContext& context)
{
	osg::Matrix modelMatrix = mTransform->getWorldMatrices().front();
	mModelMatrixUniform->set(modelMatrix);
}

} // namespace vis
} // namespace skybolt
