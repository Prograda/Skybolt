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

#include <rttr/registration>

#define SKYBOLT_ENABLE_POLYMORPHIC_REFLECTION(...) RTTR_ENABLE(__VA_ARGS__)

// FIXME: RTTR_REGISTRATION macro doesn't work with static libraries because the compiler
// optimizes out the registration functions and they don't get called. To work around this
// issue we use SKYBOLT_REFLECT* macros instead of RTTR_REGISTRATION.
// This solution is based on https://github.com/rttrorg/rttr/issues/106
#undef RTTR_REGISTRATION

//! This macro allows reflection to be defined in the header file
#define SKYBOLT_REFLECT_INLINE(cls)                                                                          \
	template <typename T>                                                                                    \
	extern void rttr_auto_register_reflection_function_t();                                                  \
	template <>                                                                                              \
	void rttr_auto_register_reflection_function_t<cls>();                                                    \
	static const int auto_register__##cls = []() {                                                           \
		rttr_auto_register_reflection_function_t<cls>();                                                     \
		return 0;                                                                                            \
	}();                                                                                                     \
	template <>                                                                                              \
	inline void rttr_auto_register_reflection_function_t<cls>()

//! This macro goes in the header file and allows reflection to be defined in the cpp file using SKYBOLT_REFLECT
#define SKYBOLT_REFLECT_EXTERN(cls)                                                                          \
	template <typename T>                                                                                    \
	extern void rttr_auto_register_reflection_function_t();                                                  \
	template <>                                                                                              \
	void rttr_auto_register_reflection_function_t<cls>();                                                    \
	static const int auto_register__##cls = []() {                                                           \
		rttr_auto_register_reflection_function_t<cls>();                                                     \
		return 0;                                                                                            \
	}();

//! This macro goes in the cpp file
#define SKYBOLT_REFLECT(cls)                                                                                 \
	template <>                                                                                              \
	void rttr_auto_register_reflection_function_t<cls>()


namespace skybolt::sim {

template <typename T>
rttr::type getType(const T& object)
{
	return rttr::type::get(object);
}

template<typename ObjectT>
rttr::property getProperty(const ObjectT& object, const std::string& propertyName)
{
	return getType(object)
		.get_property(propertyName);
}

template<typename ObjectT>
rttr::array_range<rttr::property> getProperties(const ObjectT& object)
{
	return getType(object)
		.get_properties();
}

enum class PropertyMetadataType
{
	Units
};

} // namespace skybolt::sim
