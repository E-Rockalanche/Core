#pragma once

#include <stdx/assert.h>
#include <stdx/type_traits.h>

#include <limits>

namespace Math
{

template <typename T>
struct ColorComponentTraits
{
	static_assert( std::is_arithmetic_v<T> );

	using ValueType = std::remove_cv_t<T>;

	static constexpr ValueType Max = std::is_floating_point_v<ValueType> ? T( 1 ) : std::numeric_limits<ValueType>::max();
	static constexpr ValueType Min = T( 0 );
};

template <typename T>
struct ColorRGB
{
	using ValueType = T;
	static constexpr auto Max = ColorComponentTraits<T>::Max;

	T r;
	T g;
	T b;

	constexpr ColorRGB() noexcept = default;
	constexpr ColorRGB( T red, T green, T blue ) noexcept : r( red ), g( green ), b( blue ) {}
	constexpr explicit ColorRGB( T rgb ) noexcept : r( rgb ), g( rgb ), b( rgb ) {}

	static constexpr ColorRGB Black() noexcept { return ColorRGB( 0, 0, 0 ); }
	static constexpr ColorRGB White() noexcept { return ColorRGB( Max, Max, Max ); }
	static constexpr ColorRGB Red() noexcept { return ColorRGB( Max, 0, 0 ); }
	static constexpr ColorRGB Green() noexcept { return ColorRGB( 0, Max, 0 ); }
	static constexpr ColorRGB Blue() noexcept { return ColorRGB( 0, 0, Max ); }
	static constexpr ColorRGB Yellow() noexcept { return ColorRGB( Max, Max, 0 ); }
	static constexpr ColorRGB Cyan() noexcept { return ColorRGB( 0, Max, Max ); }
	static constexpr ColorRGB Magenta() noexcept { return ColorRGB( Max, 0, Max ); }
};

template <typename T>
constexpr bool operator==( const ColorRGB<T>& lhs, const ColorRGB<T>& rhs ) noexcept
{
	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}

template <typename T>
constexpr bool operator!=( const ColorRGB<T>& lhs, const ColorRGB<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
struct ColorRGBA : ColorRGB<T>
{
	using ValueType = T;
	static constexpr auto Max = ColorComponentTraits<T>::Max;

	T a;

	constexpr ColorRGBA() noexcept = default;
	constexpr ColorRGBA( T red, T green, T blue, T alpha = Max ) noexcept : ColorRGB<T>( red, green, blue ), a( alpha ) {}
	constexpr explicit ColorRGBA( T rgb, T alpha = Max ) noexcept : ColorRGB<T>( rgb ), a( alpha ) {}
	constexpr ColorRGBA( const ColorRGB<T>& rgb, T alpha = ColorComponentTraits<T>::Max ) noexcept : ColorRGB<T>( rgb ), a( alpha ) {}

	static constexpr ColorRGBA Black() noexcept { return ColorRGBA( 0, 0, 0 ); }
	static constexpr ColorRGBA White() noexcept { return ColorRGBA( Max, Max, Max ); }
	static constexpr ColorRGBA Red() noexcept { return ColorRGBA( Max, 0, 0 ); }
	static constexpr ColorRGBA Green() noexcept { return ColorRGBA( 0, Max, 0 ); }
	static constexpr ColorRGBA Blue() noexcept { return ColorRGBA( 0, 0, Max ); }
	static constexpr ColorRGBA Yellow() noexcept { return ColorRGBA( Max, Max, 0 ); }
	static constexpr ColorRGBA Cyan() noexcept { return ColorRGBA( 0, Max, Max ); }
	static constexpr ColorRGBA Magenta() noexcept { return ColorRGBA( Max, 0, Max ); }
};

template <typename T>
constexpr bool operator==( const ColorRGBA<T>& lhs, const ColorRGBA<T>& rhs ) noexcept
{
	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

template <typename T>
constexpr bool operator!=( const ColorRGBA<T>& lhs, const ColorRGBA<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

/*
template <typename T>
struct ColorBGR
{
	using ValueType = T;
	static constexpr auto Max = ColorComponentTraits<T>::Max;

	T b;
	T g;
	T r;

	constexpr ColorBGR() noexcept = default;
	constexpr ColorBGR( T blue, T green, T red ) : b( blue ), g( green ), r( red ) {}
	constexpr explicit ColorBGR( T bgr ) : b( bgr ), g( bgr ), r( bgr ) {}
};

template <typename T>
constexpr bool operator==( const ColorBGR<T>& lhs, const ColorBGR<T>& rhs ) noexcept
{
	return lhs.b == rhs.b && lhs.g == rhs.g && lhs.r == rhs.r;
}

template <typename T>
constexpr bool operator!=( const ColorBGR<T>& lhs, const ColorBGR<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
struct ColorBGRA : ColorBGR<T>
{
	using ValueType = T;
	static constexpr auto Max = ColorComponentTraits<T>::Max;

	T a;

	constexpr ColorBGRA() noexcept = default;
	constexpr ColorBGRA( T blue, T green, T red, T alpha = Max ) : ColorBGR<T>( blue, green, red ), a( alpha ) {}
	constexpr explicit ColorBGR( T bgr, T alpha = Max ) : ColorBGR<T>( bgr ), a( alpha ) {}
	constexpr ColorBGRA( const ColorBGR<T>& bgr, T alpha = ColorComponentTraits<T>::Max ) noexcept : ColorRGB<T>( bgr ), a( alpha ) {}
};

template <typename T>
constexpr bool operator==( const ColorBGRA<T>& lhs, const ColorBGRA<T>& rhs ) noexcept
{
	return lhs.b == rhs.b && lhs.g == rhs.g && lhs.r == rhs.r && lhs.a == rhs.a;
}

template <typename T>
constexpr bool operator!=( const ColorBGRA<T>& lhs, const ColorBGRA<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}
*/

namespace Detail
{
	template <typename T>
	using AlphaType = decltype( std::declval<T>().a );
}

template <typename Color>
inline constexpr bool HasAlpha = stdx::is_detected_v<Detail::AlphaType, Color>;

template <typename To, typename From>
constexpr To ColorCast( const From& from ) noexcept
{
	constexpr auto ToMax = ColorComponentTraits<typename To::ValueType>::Max;
	constexpr auto FromMax = ColorComponentTraits<typename From::ValueType>::Max;

	To result;

	result.r = ( from.r * ToMax ) / FromMax;
	result.g = ( from.g * ToMax ) / FromMax;
	result.b = ( from.b * ToMax ) / FromMax;

	if constexpr ( HasAlpha<To> )
	{
		if constexpr ( HasAlpha<From> )
			result.a = ( from.a * ToMax ) / FromMax;
		else
			result.a = ToMax;
	}

	return result;
}

template <template <typename> typename To, typename From>
constexpr To<typename From::ValueType> ColorCast( const From& from ) noexcept
{
	To<typename From::ValueType> result;

	result.r = from.r;
	result.g = from.g;
	result.b = from.b;

	if constexpr ( HasAlpha<To> )
	{
		if constexpr ( HasAlpha<From> )
			result.a = from.a;
		else
			result.a = ColorComponentTraits<typename From::ValueType>::Max;
	}

	return result;
}

template <typename Color>
Color Invert( const Color& c )
{
	constexpr auto Max = ColorComponentTraits<typename Color::ValueType>::Max;

	Color result;
	result.r = Max - c.r;
	result.g = Max - c.g;
	result.b = Max - c.b;

	if constexpr ( HasAlpha<Color> )
		result.a = c.a;

	return result;
}

template <typename Color>
typename Color::ValueType Intensity( const Color& c )
{
	return static_cast<typename Color::ValueType>( ( c.r * 30 + c.g * 59 + c.b * 11 ) / 100 );
}

template <typename Color>
constexpr Color GreyScale( const Color& c )
{
	const auto intensity = Intensity( c );
	Color result;
	result.r = intensity;
	result.g = intensity;
	result.b = intensity;

	if constexpr ( HasAlpha<Color> )
		result.a = c.a;

	return result;
}

using ColorRGB8 = ColorRGB<uint8_t>;
using ColorRGB16 = ColorRGB<uint16_t>;
using ColorRGB32 = ColorRGB<uint32_t>;
using ColorRGBf = ColorRGB<float>;
using ColorRGBd = ColorRGB<double>;

using ColorRGBA8 = ColorRGBA<uint8_t>;
using ColorRGBA16 = ColorRGBA<uint16_t>;
using ColorRGBA32 = ColorRGBA<uint32_t>;
using ColorRGBAf = ColorRGBA<float>;
using ColorRGBAd = ColorRGBA<double>;

/*
using ColorBGR8 = ColorBGR<uint8_t>;
using ColorBGR16 = ColorBGR<uint16_t>;
using ColorBGR32 = ColorBGR<uint32_t>;
using ColorBGRf = ColorBGR<float>;
using ColorBGRd = ColorBGR<double>;

using ColorBGRA8 = ColorBGRA<uint8_t>;
using ColorBGRA16 = ColorBGRA<uint16_t>;
using ColorBGRA32 = ColorBGRA<uint32_t>;
using ColorBGRAf = ColorBGRA<float>;
using ColorBGRAd = ColorBGRA<double>;
*/

}