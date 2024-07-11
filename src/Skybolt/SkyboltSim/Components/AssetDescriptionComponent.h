/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltSim/Component.h"
#include "SkyboltSim/SkyboltSimFwd.h"
#include <assert.h>
#include <string>

namespace skybolt {
namespace sim {

struct AssetDescription
{
	std::string description;
	std::string sourceUrl;
	std::vector<std::string> authors;
};

class AssetDescriptionComponent : public sim::Component
{
public:
	AssetDescriptionComponent(const std::shared_ptr<AssetDescription>& description)
	: mDescription(description)
	{
		assert(mDescription);
	}

	const AssetDescription& getDescription() const {return *mDescription;}

private:
	std::shared_ptr<AssetDescription> mDescription;
};

} // namespace sim
} // namespace skybolt