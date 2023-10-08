/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/* Copyright 2012-2019 Matthew Paul Reid
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt::sim {

enum class AttributeType
{
	PositionInWorld,
	Orientation
};

struct PropertyMetadataNames
{
	static constexpr char* units = "Units"; //!< Value is Units enum giving the measurement units of the property
	static constexpr char* attributeType = "AttributeType"; //!< Value is AttributeType enum giving information about the physical meaning of the property values
	static constexpr char* multiLine = "MultiLine"; //!< Value is a bool indicating whether text should be rendered as multiple lines
};

} // namespace skybolt::sim
