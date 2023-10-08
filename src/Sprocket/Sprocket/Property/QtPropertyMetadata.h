/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

struct QtPropertyMetadataNames
{
    static constexpr char* attributeType = "AttributeType"; //!< Property value is Datum enum stored as an int
	static constexpr char* enumValueDisplayNames = "EnumValueDisplayNames"; //!< Property value is a QStringList containing the display names of the enum values of a property
	static constexpr char* multiLine = "MultiLine"; //!< Property value is a bool indicating whether text should be rendered as multiple lines
};