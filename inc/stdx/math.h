#ifndef STDX_MATH_HPP
#define STDX_MATH_HPP

#define _USE_MATH_DEFINES
#include <math.h>

namespace stdx
{

template <typename T>
constexpr T pi = static_cast<T>( M_PI );

template <typename T>
constexpr T half_pi = static_cast<T>( M_PI_2 );

template <typename T>
constexpr T quarter_pi = static_cast<T>( M_PI_4 );

template <typename T>
constexpr T two_pi = static_cast<T>( 2 * pi<T> );

template <typename T>
constexpr T inv_pi = static_cast<T>( M_1_PI );

template <typename T>
constexpr T two_inv_pi = static_cast<T>( M_2_PI );

template <typename T>
constexpr T two_inv_sqrt_pi = static_cast<T>( M_2_SQRTPI );

template <typename T>
constexpr T sqrt2 = static_cast<T>( M_SQRT2 );

template <typename T>
constexpr T inv_sqrt2 = static_cast<T>( M_SQRT1_2 );

template <typename T>
constexpr T e = static_cast<T>( M_E );

template <typename T>
constexpr T log2e = static_cast<T>( M_LOG2E );

template <typename T>
constexpr T log10e = static_cast<T>( M_LOG10E );

template <typename T>
constexpr T ln2 = static_cast<T>( M_LN2 );

template <typename T>
constexpr T ln10 = static_cast<T>( M_LN10 );

template <typename T>
constexpr T deg_to_rad( T deg ) noexcept { return deg * pi<T> / 180; }

template <typename T>
constexpr T rad_to_deg( T rad ) noexcept { return rad * 180 / pi<T>; }

template <typename T>
constexpr T abs( const T& value )
{
	return ( value < 0 ) ? -value : value;
}

}

#endif