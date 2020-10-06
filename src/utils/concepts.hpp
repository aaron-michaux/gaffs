
#pragma once

#include <type_traits>

#if defined __GLIBCXX__ && __GLIBCXX__ <= 20200422
namespace std
{
template<class T> concept integral        = is_integral_v<T>;
template<class T> concept signed_integral = integral<T>&& is_signed_v<T>;
template<class T>
concept unsigned_integral                = integral<T> && !signed_integral<T>;
template<class T> concept floating_point = is_floating_point_v<T>;
template<class T> concept arithmetic     = is_arithmetic_v<T>;

template<class F, class... Args>
concept invocable = requires(F&& f, Args&&... args)
{
   invoke(forward<F>(f), forward<Args>(args)...);
};

template<class F, class... Args>
concept regular_invocable = invocable<F, Args...>;

template<class T, class U> concept same_as = is_same_v<T, U>&& is_same_v<U, T>;
template<class Derived, class Base>
concept derived_from = is_base_of_v<Base, Derived>&&
    is_convertible_v<const volatile Derived*, const volatile Base*>;

template<class From, class To>
concept convertible_to = is_convertible_v<From, To>&& requires(From (&f)())
{
   static_cast<To>(f());
};

template<class T> concept destructible = is_nothrow_destructible_v<T>;
template<class T, class... Args>
concept constructible_from = destructible<T>&& is_constructible_v<T, Args...>;
template<class T>
concept move_constructible = constructible_from<T, T>&& convertible_to<T, T>;

} // namespace std
#else
#error \
    "update include guards... see if we have concepts in libstdc++, or libc++"
#endif
