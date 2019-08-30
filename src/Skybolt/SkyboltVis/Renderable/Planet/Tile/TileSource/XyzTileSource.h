/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "TileSource.h"

namespace skybolt {
namespace vis {

struct XyzTileSourceConfig
{
	//! URL must be templated with variables x, y, z in curley braces
	//! E.g "https://test.com/image/{z}/{x}/{y}.png"
	std::string urlTemplate;

	enum class YOrigin
	{
		Top, //!< y=0 tile is at the top of the image
		Bottom //!< y=0 tile is at the bottom of the image
	};

	YOrigin yOrigin = YOrigin::Top;
};

class XyzTileSource : public TileSource
{
public:
	XyzTileSource(const XyzTileSourceConfig& config);

	osg::ref_ptr<osg::Image> createImage(const skybolt::QuadTreeTileKey& key, std::function<bool()> cancelSupplier) const override;

private:
	std::string mUrlTemplate;
	XyzTileSourceConfig::YOrigin mYOrigin;
};

} // namespace vis
} // namespace skybolt
