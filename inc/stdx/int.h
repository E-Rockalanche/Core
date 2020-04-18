#pragma once

#include <stdx/assert.h>

#include <cstdint>
#include <limits>
#include <type_traits>

using u8 = uint8_t;
using s8 = int8_t;

using u16 = uint16_t;
using s16 = int16_t;

using u32 = uint32_t;
using s32 = int32_t;

using u64 = uint64_t;
using s64 = int64_t;

template <typename T>
constexpr T MaxValue = std::numeric_limits<T>::max();

template <typename T>
constexpr T MinValue = std::numeric_limits<T>::min();

template <typename To, typename From>
constexpr To narrow_cast( const From from )
{
	static_assert(
		std::is_integral_v<To> && std::is_integral_v<From>,
		"narrow_cast is for casting integral types only" );

	if constexpr ( std::is_signed_v<To> == std::is_signed_v<From> && sizeof( To ) >= sizeof( From ) )
	{
		// no further checks needed
		return static_cast<To>( from );
	}
	else
	{
		const To result = static_cast<To>( from );
		dbEnsures( static_cast<From>( result ) == from && ( ( result < 0 ) == ( from < 0 ) ) );
		return result;
	}
}