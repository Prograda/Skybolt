/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <vector>
#include <algorithm>

namespace skybolt
{

template <typename ListenerT>
class Listenable
{
public:
	typedef ListenerT Listener;

	void addListener(Listener* listener)
	{
		mListeners.push_back(listener);
	}

	void removeListener(Listener* listener)
	{
		typename Listeners::iterator i = std::find(mListeners.begin(), mListeners.end(), listener);
		if (i != mListeners.end())
			mListeners.erase(i);

	}

	typedef std::vector<Listener*> Listeners;
	Listeners mListeners;
};

// Call listeners in reverse order in case listeners are removed during callback
#define CALL_LISTENERS(fn) \
for (int i = (int)mListeners.size()-1; i >= 0; --i) \
	mListeners[i]->fn;

} // namespace skybolt
