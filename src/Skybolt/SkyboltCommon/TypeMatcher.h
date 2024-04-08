#pragma once

#include <assert.h>
#include <functional>

namespace skybolt {

/* Based on https://stackoverflow.com/questions/22822836/type-switch-construct-in-c11

	Example usage:
	```
	TypeMatcher(derived,
		[&] (const DerivedA& a) {
			...
		},
		[&] (const DerivedB& a) {
			...
		}
	);
	```
*/
template<typename T> struct remove_class { };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...)> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) const> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) volatile> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) const volatile> { using type = R(A...); };

template<typename T>
struct get_signature_impl { using type = typename remove_class<
    decltype(&std::remove_reference<T>::type::operator())>::type; };
template<typename R, typename... A>
struct get_signature_impl<R(A...)> { using type = R(A...); };
template<typename R, typename... A>
struct get_signature_impl<R(&)(A...)> { using type = R(A...); };
template<typename R, typename... A>
struct get_signature_impl<R(*)(A...)> { using type = R(A...); };
template<typename T> using get_signature = typename get_signature_impl<T>::type;

template<typename Base, typename T>
bool TypeMatcherHelper(const Base& base, std::function<void(T&)> func)
{
	if (T* first = dynamic_cast<T*>(&base); first)
	{
		func(*first);
		return true;
	}
	else
	{
		return false;
	}
}

template<typename Base>
void TypeMatcher(const Base&)
{
}

template<typename Base, typename FirstSubclass, typename... RestOfSubclasses>
void TypeMatcher(const Base& base, FirstSubclass &&first, RestOfSubclasses &&... rest)
{
	using Signature = get_signature<FirstSubclass>;
	using Function = std::function<Signature>;

	if (TypeMatcherHelper(base, (Function)first))
	{
		return;
	}
	else
	{
		TypeMatcher(base, rest...);
	}
}

} // namespace skybolt