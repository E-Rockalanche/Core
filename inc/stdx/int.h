#pragma once

#include <stdx/assert.h>
#include <stdx/bit.h>

#include <cstdint>
#include <limits>
#include <type_traits>

using uint = unsigned int;
using ushort = unsigned short;
using ulong = unsigned long;
using ullong = unsigned long long;

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

using s64 = int64_t;
using s32 = int32_t;
using s16 = int16_t;
using s8 = int8_t;

namespace stdx
{

template <typename To, typename From>
constexpr To narrow_cast( const From from ) noexcept
{
	static_assert(
		std::is_integral_v<To> && std::is_integral_v<From>,
		"narrow_cast is for casting integral types only" );

	if constexpr ( std::is_signed_v<To> == std::is_signed_v<From> && sizeof( To ) >= sizeof( From ) )
	{
		// safe to cast without checks
		return static_cast<To>( from );
	}
	else
	{
		const To result = static_cast<To>( from );
		dbEnsures( ( static_cast<From>( result ) == from ) && ( ( result < 0 ) == ( from < 0 ) ) );
		return result;
	}
}

// get int of exact size

template <size_t Bits>
struct int_exact {};

template <>
struct int_exact<8>
{
	using type = int8_t;
};

template <>
struct int_exact<16>
{
	using type = int16_t;
};

template <>
struct int_exact<32>
{
	using type = int32_t;
};

template <>
struct int_exact<64>
{
	using type = int64_t;
};

template <size_t Bits>
struct uint_exact {};

template <>
struct uint_exact<8>
{
	using type = uint8_t;
};

template <>
struct uint_exact<16>
{
	using type = uint16_t;
};

template <>
struct uint_exact<32>
{
	using type = uint32_t;
};

template <>
struct uint_exact<64>
{
	using type = uint64_t;
};

template <size_t Bits>
using int_exact_t = typename int_exact<Bits>::type;

template <size_t Bits>
using uint_exact_t = typename uint_exact<Bits>::type;

// get int of at least size

template <size_t Bits>
struct int_least
{
	using type = typename int_exact<std::max<size_t>( bit_ceil( Bits ), 8 )>::type;
};

template <size_t Bits>
struct uint_least
{
	using type = typename uint_exact<std::max<size_t>( bit_ceil( Bits ), 8 )>::type;
};

template <size_t Bits>
using int_least_t = typename int_least<Bits>::type;

template <size_t Bits>
using uint_least_t = typename uint_least<Bits>::type;

// get int of at least enough size to hold value

template <std::intmax_t V>
struct int_max_value
{
	static_assert( V >= 0, "value must be non-negative" );
	using type = typename int_least_t<static_cast<size_t>( bit_width( V ) + 1 )>::type;
};

template <std::intmax_t V>
struct int_min_value
{
	static_assert( V < 0, "value must be negative" );
	using type = typename int_least_t<static_cast<size_t>( bit_width( V ) )>::type;
};

template <std::uintmax_t V>
struct uint_value
{
	using type = typename uint_least_t<static_cast<size_t>( bit_width( V ) )>::type;
};

template <std::intmax_t V>
using int_max_value_t = typename int_max_value<V>::type;

template <std::intmax_t V>
using int_min_value_t = typename int_min_value<V>::type;

template <std::uintmax_t V>
using uint_value_t = typename uint_value<V>::type;

// promoted types

template <typename T>
struct promoted {};

template <>
struct promoted<int8_t>
{
	using type = int16_t;
};

template <>
struct promoted<int16_t>
{
	using type = int32_t;
};

template <>
struct promoted<int32_t>
{
	using type = int64_t;
};

template <>
struct promoted<uint8_t>
{
	using type = uint16_t;
};

template <>
struct promoted<uint16_t>
{
	using type = uint32_t;
};

template <>
struct promoted<uint32_t>
{
	using type = uint64_t;
};

template <typename T>
using promoted_t = typename promoted<T>::type;

template <typename T>
struct promoted_fast {};

template <>
struct promoted_fast<int8_t>
{
	using type = int_fast16_t;
};

template <>
struct promoted_fast<int16_t>
{
	using type = int_fast32_t;
};

template <>
struct promoted_fast<int32_t>
{
	using type = int_fast64_t;
};

template <>
struct promoted_fast<uint8_t>
{
	using type = uint_fast16_t;
};

template <>
struct promoted_fast<uint16_t>
{
	using type = uint_fast32_t;
};

template <>
struct promoted_fast<uint32_t>
{
	using type = uint_fast64_t;
};

template <typename T>
using promoted_fast_t = typename promoted_fast<T>::type;

} // namespace stdx

constexpr uint8_t operator ""_u8( unsigned long long n ) noexcept
{
	return stdx::narrow_cast<uint8_t>( n );
}

constexpr int8_t operator ""_s8( unsigned long long n ) noexcept
{
	return stdx::narrow_cast<int8_t>( n );
}

constexpr uint16_t operator ""_u16( unsigned long long n ) noexcept
{
	return stdx::narrow_cast<uint16_t>( n );
}

constexpr int16_t operator ""_s16( unsigned long long n ) noexcept
{
	return stdx::narrow_cast<int16_t>( n );
}

constexpr uint32_t operator ""_u32( unsigned long long n ) noexcept
{
	return stdx::narrow_cast<uint32_t>( n );
}

constexpr int32_t operator ""_s32( unsigned long long n ) noexcept
{
	return stdx::narrow_cast<int32_t>( n );
}

constexpr uint64_t operator ""_u64( unsigned long long n ) noexcept
{
	return stdx::narrow_cast<uint64_t>( n );
}

constexpr int64_t operator ""_s64( unsigned long long n ) noexcept
{
	return stdx::narrow_cast<int64_t>( n );
}