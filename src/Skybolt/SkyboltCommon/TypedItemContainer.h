/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <map>
#include <memory>
#include <typeindex>
#include <vector>
#include <algorithm>

namespace skybolt {

namespace detail {

template <typename BaseT, typename OtherT>
std::shared_ptr<OtherT> static_or_dynamic_pointer_cast(const std::shared_ptr<BaseT>& type, std::true_type)
{
    return std::static_pointer_cast<OtherT>(type);
}

template <typename BaseT, typename OtherT>
std::shared_ptr<OtherT> static_or_dynamic_pointer_cast(const std::shared_ptr<BaseT>& type, std::false_type)
{
    return std::dynamic_pointer_cast<OtherT>(type);
}

//! static_or_dynamic_pointer_cast uses a static_pointer_cast if the types are known to be related at compile time, otherwse falls back to dyanmic_pointer_cast
template <typename BaseT, typename OtherT>
std::shared_ptr<OtherT> static_or_dynamic_pointer_cast(const std::shared_ptr<BaseT>& type)
{
	return static_or_dynamic_pointer_cast<BaseT, OtherT>(type, std::disjunction<std::is_base_of<BaseT, OtherT>, std::is_same<BaseT, OtherT>>());
}

} // namespace detail

template <class T>
inline std::vector<std::type_index> getExposedTypes(const T& type)
{
	return { typeid(type) };
}

template <typename BaseT>
class TypedItemContainer
{
	typedef std::shared_ptr<BaseT> BaseTPtr;

public:
	virtual ~TypedItemContainer()
	{
		mComponentMap.clear();
		// Delete components in reverse order
		for (int i = (int)mComponents.size() - 1; i >= 0; --i)
		{
			mComponents[i].reset();
		}
	}

	virtual void addItem(const BaseTPtr& c)
	{
		mComponents.push_back(c);

		for (const auto& type : getExposedTypes(*c))
		{
			mComponentMap.insert(typename ComponentMap::value_type(type, c));
		}
	}

	virtual void removeItem(const BaseTPtr& c)
	{
		{
			auto it = std::find(mComponents.begin(), mComponents.end(), c);
			if (it != mComponents.end())
			{
				mComponents.erase(it);
			}
		}

		for (auto it = mComponentMap.begin(); it != mComponentMap.end();)
		{
			if (it->second == c)
			{
				it = mComponentMap.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	//! @returns nullptr if not found
	template <class DerivedT>
	std::vector<std::shared_ptr<DerivedT>> getItemsOfType() const
	{
		std::vector<std::shared_ptr<DerivedT> > result;

		std::type_index index = typeid(DerivedT);
		typename ComponentMap::const_iterator it = mComponentMap.lower_bound(index);
		typename ComponentMap::const_iterator it2 = mComponentMap.upper_bound(index);

		while (it != it2)
		{
			if (const auto& p = detail::static_or_dynamic_pointer_cast<BaseT, DerivedT>(it->second); p)
			{
				result.push_back(p);
			}
			++it;
		}

		return result;
	}

	inline std::vector<BaseTPtr> getAllItems() const
	{
		return mComponents;
	}

	//! @returns nullptr if not found
	template <class DerivedT>
	std::shared_ptr<DerivedT> getFirstItemOfType() const
	{
		typename ComponentMap::const_iterator i = mComponentMap.find(typeid(DerivedT));
		if (i != mComponentMap.end())
		{
			return detail::static_or_dynamic_pointer_cast<BaseT, DerivedT>(i->second);
		}

		return nullptr;
	}

protected:
	std::vector<BaseTPtr> mComponents;

private:
	typedef std::multimap<std::type_index, BaseTPtr> ComponentMap;
	ComponentMap mComponentMap;

};

} // namespace skybolt