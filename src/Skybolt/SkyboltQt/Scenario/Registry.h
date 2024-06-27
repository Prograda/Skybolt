/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "SkyboltQt/SkyboltQtFwd.h"

#include <SkyboltCommon/Listenable.h>

#include <assert.h>
#include <map>
#include <set>
#include <string>

template <typename T>
struct RegistryListener
{
	virtual void itemAdded(const std::shared_ptr<T>& item) {};
	virtual void itemAboutToBeRemoved(const std::shared_ptr<T>& item) {};
	virtual void itemRemoved(const std::string& name) {}
};

template <typename T>
class Registry : public skybolt::Listenable<RegistryListener<T>>
{
public:
	typedef std::shared_ptr<T> ItemPtr;

	void add(const ItemPtr& item)
	{
		mItems.insert(item);
		CALL_LISTENERS(itemAdded(item));
	}

	void remove(const T* item)
	{
		auto it = std::find_if(mItems.begin(), mItems.end(),
			[item](const ItemPtr& i) { return i.get() == item; });
			
		assert(it != mItems.end());

		std::string name = item->getUniqueName();

		CALL_LISTENERS(itemAboutToBeRemoved(*it));
		mItems.erase(it);

		CALL_LISTENERS(itemRemoved(name));
	}

	void clear()
	{
		std::vector<std::string> names;
		for (const ItemPtr& item : mItems)
		{
			names.push_back(item->getUniqueName());
			CALL_LISTENERS(itemAboutToBeRemoved(item));
		}
		mItems.clear();

		for (const std::string& name : names)
		{
			CALL_LISTENERS(itemRemoved(name));
		}
	}

	ItemPtr findByName(const std::string& name) const
	{
		for (const ItemPtr& item : mItems)
		{
			if (item->getUniqueName() == name)
			{
				return item;
			}
		}
		return nullptr;
	}

	const std::set<ItemPtr>& getItems() const
	{
		return mItems;
	}

	std::string createUniqueItemName(const std::string& base) const
	{
		int i = 1;
		while (true)
		{
			std::string candidate = base + std::to_string(i);
			if (!findByName(candidate))
			{
				return candidate;
			}
			++i;
		}
	}

private:
	std::set<std::shared_ptr<T>> mItems;
};
