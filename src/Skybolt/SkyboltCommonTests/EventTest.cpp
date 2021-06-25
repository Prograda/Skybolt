/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include <catch2/catch.hpp>
#include <SkyboltCommon/Event.h>

using namespace skybolt;

struct DummyEventListener : public EventListener
{
	void onEvent(const Event& event)
	{
		receivedEvent = &event;
	}
	const Event* receivedEvent = nullptr;
};

struct EventTypeA : public Event
{
};

struct EventTypeB : public Event
{
};

TEST_CASE("EventListener receives registered event type")
{
	DummyEventListener listener;

	EventEmitter emitter;
	emitter.addEventListener<EventTypeA>(&listener);

	EventTypeA event;
	emitter.emitEvent(event);

	CHECK(listener.receivedEvent == &event);
}

TEST_CASE("EventListener does not receive non-registered event type")
{
	DummyEventListener listener;

	EventEmitter emitter;
	emitter.addEventListener<EventTypeA>(&listener);

	EventTypeB event;
	emitter.emitEvent(event);

	CHECK(listener.receivedEvent == nullptr);
}
