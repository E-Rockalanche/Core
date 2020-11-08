#pragma once

#include <stdx/concepts.h>

namespace stdx::ranges
{

template <typename T>
constexpr auto begin( T&& t ) { return std::begin( t ); }

template <typename T>
constexpr auto cbegin( T&& t ) { return std::cbegin( t ); }

template <typename T>
constexpr auto end( T&& t ) { return std::end( t ); }

template <typename T>
constexpr auto cend( T&& t ) { return std::cend( t ); }

template <typename T>
constexpr auto rbegin( T&& t ) { return std::rbegin( t ); }

template <typename T>
constexpr auto crbegin( T&& t ) { return std::crbegin( t ); }

template <typename T>
constexpr auto rend( T&& t ) { return std::rend( t ); }

template <typename T>
constexpr auto crend( T&& t ) { return std::crend( t ); }

template <typename T>
constexpr auto size( T&& t ) { return std::size( t ); }

template <typename T>
constexpr auto ssize( T&& t ) { return static_cast<std::make_signed_t<decltype( std::size( std::declval<T>() ) )>>( std::size( t ) ); }

template <typename T>
constexpr auto empty( T&& t ) { return std::empty( t ); }

template <typename T>
constexpr auto data( T&& t ) { return std::data( t ); }

template <typename T>
constexpr auto cdata( T&& t ) { return static_cast<std::add_pointer_t<std::add_const_t<std::remove_pointer_t<decltype( std::data( std::declval<T>() ) )>>>>( std::data( t ) ); }

template <typename T>
STDX_concept range = stdx::is_detected_v<detail::begin_t, T> && stdx::is_detected_v<detail::end_t, T>;

template <typename T>
using iterator_t = decltype( begin( std::declval<T&>() ) );

template <typename R, STDX_requires( range<R> )
using sentinel_t = decltype( end( std::declval<R&>() ) );

}