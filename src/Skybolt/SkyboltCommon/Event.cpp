/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Event.h"
#include <boost/foreach.hpp>

namespace skybolt {

EventEmitter::~EventEmitter()
{
	for(const ListenerMap::value_type& v : mListenerMap)
	{
		for(EventListener* listener : v.second)
			listener->_removeEmitter(this);
	}
}

void EventEmitter::removeEventListener(EventListener* listener)
{
	ListenerMap::iterator e = mListenerMap.end();
	for (ListenerMap::iterator it = mListenerMap.begin(); it != e;) // Iterate all EventIds with registered listeners
	{
		it->second.erase(listener); // Remove listener if it was registered to this EventId
		if (it->second.empty()) // Remove EventId if there are no registered listeners left
		{
			ListenerMap::iterator killIt = it;
			++it;
			mListenerMap.erase(killIt);
		}
		else
		{
			++it;
		}
	}

	if (!listener->mDestroying)
		listener->_removeEmitter(this);
}

EventListener::EventListener() : mDestroying(false)
{}

EventListener::~EventListener()
{
	mDestroying = true;
	for(EventEmitter* emitter : mEmitters)
		emitter->removeEventListener(this);
}

void EventListener::_addEmitter(EventEmitter* emitter)
{
	mEmitters.insert(emitter);
}

void EventListener::_removeEmitter(EventEmitter* emitter)
{
	mEmitters.erase(emitter);
}

} // namespace skybolt