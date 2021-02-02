#pragma once

#include <stdx/type_traits.h>

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

namespace detail
{
	template <typename T, typename Diff>
	using advance_type_t = decltype( std::declval<T>() + std::declval<Diff>() );
}

template <typename T, typename Diff, STDX_requires( std::is_integral_v<Diff> )
constexpr T advance( T iterator, Diff diff ) noexcept
{
	if constexpr ( stdx::is_detected_v<detail::advance_type_t, T, Diff> )
	{
		return iterator + diff;
	}
	else
	{
		for ( ; diff > 0; --diff )
			++iterator;

		if constexpr ( std::is_signed_v<Diff> )
		{
			for ( ; diff < 0; ++diff )
				--iterator;
		}

		return iterator;
	}
}

}