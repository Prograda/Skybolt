#pragma once

#include <assert.h>

namespace skybolt {
	
//! Pointer that is non-null and refers to an object that always exists for the lifetime
//! of the pointer. 
template<typename T>
class NonNullPtr
{
public:
    NonNullPtr(T* ptr) : mPtr(ptr)
	{
        assert(mPtr != nullptr);
    }

    // Allow conversion to const types
    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    NonNullPtr(const NonNullPtr<U>& other) noexcept : mPtr(other.get()) {}

    // Accessors
    T* get() const noexcept {
        assert(mPtr && "NotNullPtr should never be null");
        return mPtr;
    }

    T& operator*() const noexcept {
        assert(mPtr);
        return *mPtr;
    }

    T* operator->() const noexcept {
        assert(mPtr);
        return mPtr;
    }

    // Implicit conversion to T*
    operator T* () const noexcept {
        assert(mPtr);
        return mPtr;
    }

    // Comparison operators
    friend bool operator==(const NonNullPtr& lhs, const NonNullPtr& rhs) noexcept {
        return lhs.mPtr == rhs.mPtr;
    }
    friend bool operator!=(const NonNullPtr& lhs, const NonNullPtr& rhs) noexcept {
        return !(lhs == rhs);
    }
	
private:
    T* mPtr;
};
	
} // namespace skybolt