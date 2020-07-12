#ifndef STDX_BIT_HPP
#define STDX_BIT_HPP

#include <stdx/assert.h>
#include <stdx/type_traits.h>
#include <cstdint>
#include <limits>

namespace stdx
{

template<typename E>
constexpr std::size_t bit_sizeof() noexcept
{
	return sizeof( E ) * 8;
}

template<typename E>
constexpr std::size_t bit_sizeof( const E& ) noexcept
{
	return bit_sizeof<E>();
}

namespace detail
{
	template <typename To, typename From>
	struct bit_cast_union
	{
		constexpr bit_cast_union( const From& src ) noexcept : from{ src } {}

		union
		{
			To to;
			From from;
		};
	};
}

enum class endian
{
#ifdef _WIN32
    little = 0,
    big    = 1,
    native = little
#else
    little = __ORDER_LITTLE_ENDIAN__,
    big    = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__
#endif
};

template <typename To, typename From,
	std::enable_if_t<( sizeof( To ) == sizeof( From ) ) &&
	std::is_trivially_copyable_v<From> &&
	std::is_trivial_v<To>, int> = 0>
constexpr To bit_cast( const From& src ) noexcept
{
	detail::bit_cast_union<To, From> caster{ src };
	return caster.to;
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
constexpr bool has_single_bit( T x ) noexcept
{
	return ( x != 0 ) && ( x & ( x - 1 ) ) == 0;
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
constexpr int countl_zero( T x ) noexcept
{
	int count = 0;
	for( T bit = 1 << ( std::numeric_limits<T>::digits - 1 ); bit != 0; bit >>= 1 )
	{
		if ( x & bit )
			break;
		else
			++count;
	}
	return count;
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
constexpr int countl_one( T x ) noexcept
{
	int count = 0;
	for( T bit = 1 << ( std::numeric_limits<T>::digits - 1 ); bit != 0; bit >>= 1 )
	{
		if ( x & bit )
			++count;
		else
			break;
	}
	return count;
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
constexpr int countr_zero( T x ) noexcept
{
	int count = 0;
	for( T bit = 1; bit != 0; bit <<= 1 )
	{
		if ( x & bit )
			break;
		else
			++count;
	}
	return count;
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
constexpr int countr_one( T x ) noexcept
{
	int count = 0;
	for( T bit = 1; bit != 0; bit <<= 1 )
	{
		if ( x & bit )
			++count;
		else
			break;
	}
	return count;
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
constexpr int popcount( T x ) noexcept
{
	int count = 0;
	for( T bit = 1; bit != 0; bit <<= 1 )
	{
		if ( x & bit )
			++count;
	}
	return count;
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
constexpr T bit_width( T x ) noexcept
{
	return std::numeric_limits<T>::digits - stdx::countl_zero( x );
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
constexpr T bit_ceil( T x ) noexcept
{
	if ( has_single_bit( x ) )
		return x;

	const T result = 1 << bit_width( x );
	dbAssert( result != 0 ); // check overflow
	return result;
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
constexpr T bit_floor( T x ) noexcept
{
	return ( x == 0 )
		? 0
		: ( has_single_bit( x ) )
			? x
			: 1 << ( bit_width( x ) - 1 );
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
[[nodiscard]] constexpr T rotl( T x, int s ) noexcept
{
	constexpr auto N = std::numeric_limits<T>::digits;
	const auto r = s % N;

	if ( r == 0 )
		return x;
	else if ( r > 0 )
		return ( x << r ) | ( x >> ( N - r ) );
	else
		return ( x >> -r ) | ( x << ( N + r ) );
}

template <typename T,
	std::enable_if_t<stdx::is_unsigned_integral_v<T>, int> = 0>
[[nodiscard]] constexpr T rotr( T x, int s ) noexcept
{
	constexpr auto N = std::numeric_limits<T>::digits;
	const auto r = s % N;

	if ( r == 0 )
		return x;
	else if ( r > 0 )
		return ( x >> r ) | ( x << ( N - r ) );
	else if ( r < 0 )
		return ( x << -r ) | ( x >> ( N + r ) );
}

constexpr uintmax_t unsigned_bits_max( size_t bits ) noexcept
{
	dbExpects( bits > 0 );
	const uintmax_t result = ( uintmax_t{ 1 } << bits ) - 1;
	dbEnsures( result != 0 );
	return result;
}

constexpr intmax_t signed_bits_max( size_t bits ) noexcept
{
	dbExpects( bits > 0 );
	const intmax_t result = ( intmax_t{ 1 } << ( bits - 1 ) ) - 1;
	dbEnsures( result != 0 || bits == 1 );
	return static_cast<int64_t>( result );
}

constexpr intmax_t signed_bits_min( size_t bits ) noexcept
{
	dbExpects( bits > 0 );
	const intmax_t result = ( intmax_t{ 1 } << ( bits - 1 ) );
	dbEnsures( result != 0 );
	return static_cast<int64_t>( result );
}

}

#endif