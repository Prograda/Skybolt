/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

struct QtPropertyMetadataNames
{
    static constexpr const char* attributeType = "AttributeType"; //!< Property value is Datum enum stored as an int
	static constexpr const char* multiLine = "MultiLine"; //!< Property value is a bool indicating whether text should be rendered as multiple lines
	static constexpr const char* optionNames = "OptionNames"; //!< Property value is a QStringList containing the names of the possible values the property can hold. Useful for representing enum types.
	static constexpr const char* allowCustomOptions = "AllowCustomOptions"; //!< Value is a bool indicating whether the "OptionNames" are exhaustive, or whether the user can set a custom value not contained in "OptionNames". False by default.
};