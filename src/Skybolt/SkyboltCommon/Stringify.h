#pragma once

/*
	Macros to convert a macro to a string.
	From: https://stackoverflow.com/questions/20631922/expand-macro-inside-string-literal

	Usage:
	"The string value of MY_MACRO is " STRINGIFY(MY_MACRO) "."
*/
#define STRINGIFY(a) STRINGIFY2(a)
#define STRINGIFY2(a) #a