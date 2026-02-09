/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace skybolt {
	
//! Pointer that refers to an object that always exists for the lifetime of the pointer. 
template<typename T>
class NullablePtr
{
public:
    NullablePtr(T* ptr) : mPtr(ptr)
	{
    }

    NullablePtr() : mPtr(nullptr)
    {
    }

    // Allow conversion to const types
    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    NullablePtr(const NullablePtr<U>& other) noexcept : mPtr(other.get()) {}

    // Accessors
    T* get() const noexcept {
        return mPtr;
    }

    T& operator*() const noexcept {
        return *mPtr;
    }

    T* operator->() const noexcept {
        return mPtr;
    }

    // Implicit conversion to T*
    operator T* () const noexcept {
        return mPtr;
    }

    // Comparison operators
    friend bool operator==(const NullablePtr& lhs, const NullablePtr& rhs) noexcept {
        return lhs.mPtr == rhs.mPtr;
    }
    friend bool operator!=(const NullablePtr& lhs, const NullablePtr& rhs) noexcept {
        return !(lhs == rhs);
    }
	
private:
    T* mPtr;
};
	
} // namespace skybolt