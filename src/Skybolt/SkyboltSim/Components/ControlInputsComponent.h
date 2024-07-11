/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltSim/Component.h>
#include <SkyboltSim/SkyboltSimFwd.h>
#include <SkyboltCommon/Range.h>
#include <boost/variant.hpp>
#include <glm/glm.hpp>
#include <memory>

namespace skybolt {
namespace sim {

struct ControlInput
{
	virtual ~ControlInput() {}
};

template <typename T>
struct ControlInputT : public ControlInput
{
	T value;
	RangeInclusive<T> range;
};

template <typename T>
T getUnitNormalized(const ControlInputT<T>& input)
{
	return (input.value - input.range.first) / (input.range.last - input.range.first);
}

template <typename T>
void setUnitNormalized(ControlInputT<T>& input, const T& value)
{
	input.value = glm::mix(input.range.first, input.range.last, value);
}

using ControlInputFloat = ControlInputT<float>;
using ControlInputVec2 = ControlInputT<glm::vec2>;

using ControlInputPtr = std::shared_ptr<ControlInput>;
using ControlInputFloatPtr = std::shared_ptr<ControlInputFloat>;
using ControlInputVec2Ptr = std::shared_ptr<ControlInputVec2>;

template <typename T>
inline RangeInclusive<T> unitRange() { return RangeInclusive<T>(T(0), T(1)); }

template <typename T>
inline RangeInclusive<T> posNegUnitRange() { return RangeInclusive<T>(T(-1), T(1)); }

class ControlInputsComponent : public Component
{
public:
	std::map<std::string, ControlInputPtr> controls;

	template <typename T>
	inline std::shared_ptr<ControlInputT<T>> get(const std::string& name) const
	{
		auto i = controls.find(name);
		if (i != controls.end())
		{
			return std::dynamic_pointer_cast<ControlInputT<T>>(i->second);
		}
		return nullptr;
	}

	template <typename T>
	void setIfPresent(const std::string& controlName, const T& value)
	{
		auto input = get<T>(controlName);
		if (input)
		{
			input->value = value;
		}
	}

	template <typename T>
	inline std::shared_ptr<ControlInputT<T>> createOrGet(const std::string& name, const T& initialValue, const RangeInclusive<T>& range = unitRange<T>())
	{
		auto& control = controls[name];
		if (!control)
		{
			auto controlT = std::make_shared<ControlInputT<T>>();
			controlT->value = initialValue;
			controlT->range = range;
			control = controlT;
			return controlT;
		}
		else
		{
			auto controlT = std::dynamic_pointer_cast<ControlInputT<T>>(control);
			if (controlT)
			{
				return controlT;
			}
		}
		throw std::runtime_error("Control Input '" + name + "' had unexpected type");
	}
};

} // namespace sim
} // namespace skybolt