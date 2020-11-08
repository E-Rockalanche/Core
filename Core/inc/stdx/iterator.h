#pragma once

#include <stdx/iterator/basic_iterator.h>
#include <stdx/iterator/zip_iterator.h>

#include <iterator>

namespace stdx
{

template <typename T>
constexpr T* to_address( T* p ) noexcept
{
	static_assert( !std::is_function_v<T> );
	return p;
}

template <typename T>
constexpr auto to_address( const T& p ) noexcept
{
	return to_address( p.operator->() );
}

template <typename T>
constexpr auto size_bytes( const T& container ) noexcept
{
	return container.size() * sizeof( std::remove_pointer_t<decltype( container.data() )> );
}

} // namespace stdx