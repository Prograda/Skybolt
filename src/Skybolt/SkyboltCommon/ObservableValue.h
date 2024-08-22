/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <boost/signals2.hpp>

namespace skybolt {

template <typename T>
class ObservableValue
{
public:
	ObservableValue(const T& value) : mValue(value) {}

	const T& get() const { return mValue; }

	void set(const T& value)
	{
		if (mValue != value)
		{
			T oldValue = mValue;
			mValue = value;
			valueChanged(oldValue, mValue);
		}
	}

	const T& operator() () const { return mValue; }

	void operator= (const T& newValue) { set(newValue); }

	using ValueChangedSignal = boost::signals2::signal<void(const T& oldValue, const T& newValue)>;
	mutable ValueChangedSignal valueChanged;

private:
	T mValue;
};

template <typename T>
boost::signals2::scoped_connection connectObservableValue(ObservableValue<T>* source, typename ObservableValue<T>::ValueChangedSignal::slot_function_type fn)
{
	boost::signals2::scoped_connection connection;
	if (source)
	{
		return source->valueChanged.connect(std::move(fn));
	}
	return connection;
}

using ObservableValueD = ObservableValue<double>;

} // namespace skybolt
