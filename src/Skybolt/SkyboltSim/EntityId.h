/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {
namespace sim {

struct EntityId
{
	std::uint32_t applicationId;
	std::uint32_t entityId;
};

// TODO: replace these operators with <=> after upgrading to c++20
inline bool operator < (const EntityId& a, const EntityId& b)
{
	return std::tie(a.applicationId, a.entityId) < std::tie(b.applicationId, b.entityId);
}

inline bool operator == (const EntityId& a, const EntityId& b)
{
	return std::tie(a.applicationId, a.entityId) == std::tie(b.applicationId, b.entityId);
}

inline bool operator != (const EntityId& a, const EntityId& b)
{
	return std::tie(a.applicationId, a.entityId) != std::tie(b.applicationId, b.entityId);
}

constexpr EntityId nullEntityId() { return {}; }

} // namespace sim
} // namespace skybolt