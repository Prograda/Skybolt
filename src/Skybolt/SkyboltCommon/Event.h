/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <vector>
#include <map>
#include <memory>
#include <set>
#include <typeindex>

namespace skybolt {

class Event
{
public:
	virtual ~Event() {}
};

class EventEmitter;

//! Receives Events emitted by EventEmitter
class EventListener
{
public:
	EventListener();
	virtual ~EventListener();

	virtual void onEvent(const Event&) = 0;

private:
	void _addEmitter(EventEmitter*);
	void _removeEmitter(EventEmitter*);

	std::set<EventEmitter*> mEmitters;
	bool mDestroying;

	friend class EventEmitter;
};

//! Class for emitting events which can be received by EventListener
class EventEmitter
{
public:
	virtual ~EventEmitter();

	template <class EventT>
	void addEventListener(EventListener* listener)
	{
		listener->_addEmitter(this);
		mListenerMap[typeid(EventT)].insert(listener);
	}

	//! Call this to explicitally remove a listener.
	//! Otherwise listener will be removed automatically when the listener is destroyed.
	void removeEventListener(EventListener*);

	template <class EventT>
	void emitEvent(const EventT& event) const
	{
		auto it = mListenerMap.find(typeid(event));
		if (it != mListenerMap.end())
		{
			// Take copy of listeners before calling onEvent(),
			// incase the listeners list changes as a result of onEvent().
			std::set<EventListener*> listeners = it->second;
			for (const auto& item : listeners)
			{
				item->onEvent(event);
			}
		}
	}

private:
	typedef std::set<EventListener*> EventListeners;
	typedef std::map<std::type_index, EventListeners> ListenerMap;
	ListenerMap mListenerMap;
};

} // namespace skybolt