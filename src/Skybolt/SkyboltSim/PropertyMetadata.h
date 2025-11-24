/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt::sim {

struct PropertyRepresentations
{
	static constexpr const char* worldPosition = "WorldPosition";
	static constexpr const char* worldOrientation = "WorldOrientation";
	static constexpr const char* toggleButton = "ToggleButton"; //!< Representation for a boolean property indicating it should be represented as a toggle button.
};

struct PropertyMetadataNames
{
	static constexpr const char* units = "Units"; //!< Value is Units enum giving the measurement units of the property
	static constexpr const char* attributeType = "Representation"; //!< Value is const char* from PropertyRepresentations, giving information about how properties should be represented in the UI
	static constexpr const char* multiLine = "MultiLine"; //!< Value is a bool indicating whether text should be rendered as multiple lines
	static constexpr const char* optionNames = "OptionNames"; //!< Value is a vector of strings giving names of values the property can hold. Useful for representing enum types.
	static constexpr const char* allowCustomOptions = "AllowCustomOptions"; //!< Value is a bool indicating whether the "OptionNames" are exhaustive, or whether the user can set a custom value not contained in "OptionNames". False by default
};

} // namespace skybolt::sim
