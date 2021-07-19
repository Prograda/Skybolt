/* Copyright 2012-2021 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

/*
	Macros to convert a macro to a string.
	From: https://stackoverflow.com/questions/20631922/expand-macro-inside-string-literal

	Usage:
	"The string value of MY_MACRO is " STRINGIFY(MY_MACRO) "."
*/
#define STRINGIFY(a) STRINGIFY2(a)
#define STRINGIFY2(a) #a