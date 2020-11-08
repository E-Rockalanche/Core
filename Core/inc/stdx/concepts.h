#pragma once

#include "type_traits.h"

#if ( __cplusplus <= 201703L )
	// c++17
#include <type_traits>
#define STDX_requires( condition ) std::enable_if_t<( condition ), int> = 0>
#define STDX_concept inline constexpr bool
#else
	// c++20
#define STDX_requires( condition ) > requires( condition )
#define STDX_concept concept
#endif

namespace stdx
{

template <typename T, typename U>
STDX_concept same_as = std::is_same_v<T, U> && std::is_same_v<U, T>;

template <typename Derived, typename Base>
STDX_concept derived_from = std::is_base_of_v<Base, Derived> && std::is_convertible_v<const volatile Derived*, const volatile Base*>;

namespace detail
{
template <typename From, typename To>
using convert_t = decltype( static_cast<To>( std::declval<From&&>() ) );
}

template <typename From, typename To>
STDX_concept convertible_to = std::is_convertible_v<From, To> && stdx::is_detected_v<detail::convert_t, From, To>;

template <typename T>
STDX_concept integral = std::is_integral_v<T>;

template <typename T>
STDX_concept signed_integral = std::is_integral_v<T> && std::is_signed_v<T>;

template <typename T>
STDX_concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
STDX_concept floating_point = std::is_floating_point_v<T>;

namespace detail
{
	template <typename LHS, typename RHS>
	using assign_t = decltype( std::declval<LHS>() = std::forward<RHS>( std::declval<RHS&&>() ) );
}

template <typename LHS, typename RHS>
STDX_concept assignable_from = std::is_lvalue_reference_v<LHS> && stdx::is_detected_v<detail::assign_t, LHS, RHS>;

namespace detail
{
	template <typename T, typename U>
	using swap_t = decltype( std::swap( std::declval<T>(), std::declval<U>() ) );
}

template <typename T>
STDX_concept swappable = stdx::is_detected_v<detail::swap_t, T&, T&>;

template <typename T, typename U>
STDX_concept swappable_with = stdx::is_detected_v<detail::swap_t, T&&, T&&> &&
	stdx::is_detected_v<detail::swap_t, T&&, U&&> &&
	stdx::is_detected_v<detail::swap_t, U&&, T&&> &&
	stdx::is_detected_v<detail::swap_t, U&&, U&&>;

template <typename T>
STDX_concept destructible = std::is_nothrow_destructible_v<T>;

template <typename T, typename... Args>
STDX_concept constructible_from = destructible<T> && std::is_constructible_v<T, Args...>;

namespace detail
{
	template <typename T>
	using value_init_t = decltype( T() );

	template <typename T>
	using direct_list_from_empty_init_t = decltype( T{} );

	template <typename T>
	inline auto return_default_init() { T t; return t; }

	template <typename T>
	using default_init_t = decltype( return_default_init<T>() );
}

template <typename T>
STDX_concept default_initializable = constructible_from<T> &&
	stdx::is_detected_v<detail::value_init_t, T> &&
	stdx::is_detected_v<detail::direct_list_from_empty_init_t, T> &&
	stdx::is_detected_v<detail::default_init_t, T>;

template <typename T>
STDX_concept move_constructible = constructible_from<T, T> && convertible_to<T, T>;

template <typename T>
STDX_concept copy_constructible = move_constructible<T> &&
	constructible_from<T, T&> && convertible_to<T&, T> &&
	constructible_from<T, const T&> && convertible_to<const T&, T> &&
	constructible_from<T, const T> && convertible_to<const T, T>;

template <typename T>
STDX_concept boolean_testable = convertible_to<T, bool>;

namespace detail
{

template <typename T, typename U>
using equal_t = decltype( std::declval<T>() == std::declval<U>() );

template <typename T, typename U>
using not_equal_t = decltype( std::declval<T>() != std::declval<U>() );

template <typename T, typename U>
STDX_concept weakly_equality_comparable_with = stdx::is_detected_v<equal_t, T, U> &&
	stdx::is_detected_v<equal_t, U, T> &&
	stdx::is_detected_v<not_equal_t, T, U> &&
	stdx::is_detected_v<not_equal_t, U, T>;

template <typename T, typename U>
using lesser_t = decltype( std::declval<T>() < std::declval<U>() );

template <typename T, typename U>
using greater_t = decltype( std::declval<T>() > std::declval<U>() );

template <typename T, typename U>
using lesser_equal_t = decltype( std::declval<T>() <= std::declval<U>() );

template <typename T, typename U>
using greater_equal_t = decltype( std::declval<T>() >= std::declval<U>() );

} // namespace detail

template <typename T>
STDX_concept equality_comparable = detail::weakly_equality_comparable_with<T, T>;

template <typename T, typename U>
STDX_concept equality_comparable_with = equality_comparable<T> && equality_comparable<U> &&  detail::weakly_equality_comparable_with<T, U>;

template <typename T>
STDX_concept totally_ordered = equality_comparable<T> &&
	stdx::is_detected_v<detail::lesser_t, T, T> &&
	stdx::is_detected_v<detail::greater_t, T, T> &&
	stdx::is_detected_v<detail::lesser_equal_t, T, T> &&
	stdx::is_detected_v<detail::greater_equal_t, T, T>;

template <typename T, typename U>
STDX_concept totally_ordered_with = totally_ordered<T> && totally_ordered<U> && equality_comparable_with<T, U> &&
	stdx::is_detected_v<detail::lesser_t, T, U> &&
	stdx::is_detected_v<detail::greater_t, T, U> &&
	stdx::is_detected_v<detail::lesser_equal_t, T, U> &&
	stdx::is_detected_v<detail::greater_equal_t, T, U> &&
	stdx::is_detected_v<detail::lesser_t, U, T> &&
	stdx::is_detected_v<detail::greater_t, U, T> &&
	stdx::is_detected_v<detail::lesser_equal_t, U, T> &&
	stdx::is_detected_v<detail::greater_equal_t, U, T>;

template <typename T>
STDX_concept movable = std::is_object_v<T> && move_constructible<T> && assignable_from<T&, T> && swappable<T>;

template <typename T>
STDX_concept copyable = copy_constructible<T> &&
	movable<T> &&
	assignable_from<T&, T&> &&
	assignable_from<T&, const T&> &&
	assignable_from<T&, const T>;

template <typename T>
STDX_concept semiregular = copyable<T> && default_initializable<T>;

template <typename T>
STDX_concept regular = semiregular<T> && equality_comparable<T>;

namespace detail
{
	template <typename F, typename Args>
	using const_invoke_result_t = decltype( std::invoke( std::declval<const F&>(), std::declval<const Args&>()... ) );
}

template <typename F, typename... Args>
STDX_concept invocable = stdx::is_detected_v<std::invoke_result_t, F, Args...>; // can modify F and Args

template <typename F, typename... Args>
STDX_concept regular_invocable = stdx::is_detected_v<detail::const_invoke_result_t, F, Args...>; // cannot modify F or Args

template <typename F, typename... Args>
STDX_concept predicate = regular_invocable<F, Args...> && boolean_testable<std::invoke_result_t<F, Args...>>;

template <typename R, typename T, typename U>
STDX_concept relation = predicate<R, T, T> && predicate<R, U, U> && predicate<R, T, U> && predicate<R, U, T>;

template <typename R, typename T, typename U>
STDX_concept equivalnce_relation = relation<R, T, U>;

template <typename R, typename T, typename U>
STDX_concept strict_weak_order = relation<R, T, U>;

}