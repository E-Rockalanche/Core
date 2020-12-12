#pragma once

#include <stdx/algorithm.h>
#include <stdx/assert.h>

#include <limits>
#include <type_traits>
#include <utility>

namespace Math
{

template <typename T>
struct ColourRGB
{
	static_assert( std::is_arithmetic_v<T>, "T must be a numeric type" );

public:
	using value_type = T;
	using size_type = std::size_t;

	static constexpr T Max = std::is_floating_point_v<T> ? static_cast<T>( 1 ) : std::numeric_limits<T>::max();
	static constexpr T Zero = static_cast<T>( 0 );

	// construction

	ColourRGB() noexcept = default;

	constexpr ColourRGB( T red, T green, T blue ) noexcept
		: r{ red }
		, g{ green }
		, b{ blue }
	{}

	constexpr explicit ColourRGB( T rgb ) noexcept
		: r{ rgb }
		, g{ rgb }
		, b{ rgb }
	{}

	// access

	constexpr T& operator[]( size_type index ) noexcept
	{
		dbExpects( index < 3 );
		return ( &r )[ index ];
	}

	constexpr const T& operator[]( size_type index ) const noexcept
	{
		dbExpects( index < 3 );
		return ( &r )[ index ];
	}

	// comparison

	friend constexpr bool operator==( const ColourRGB& lhs, const ColourRGB& rhs ) noexcept
	{
		return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
	}

	friend constexpr bool operator!=( const ColourRGB& lhs, const ColourRGB& rhs ) noexcept
	{
		return !( lhs == rhs );
	}

	// basic colours

	static constexpr ColourRGB Black() noexcept { return ColourRGB( Zero ); }
	static constexpr ColourRGB White() noexcept { return ColourRGB( Max ); }

	static constexpr ColourRGB DarkGrey() noexcept { return ColourRGB( Max / 4 ); }
	static constexpr ColourRGB Grey() noexcept { return ColourRGB( Max / 2 ); }
	static constexpr ColourRGB LightGrey() noexcept { return ColourRGB( ( Max / 4 ) * 3 ); }

	static constexpr ColourRGB Red() noexcept { return ColourRGB( Max, Zero, Zero ); }
	static constexpr ColourRGB Green() noexcept { return ColourRGB( Zero, Max, Zero ); }
	static constexpr ColourRGB Blue() noexcept { return ColourRGB( Zero, Zero, Max ); }

	static constexpr ColourRGB Yellow() noexcept { return ColourRGB( Max, Max, Zero ); }
	static constexpr ColourRGB Cyan() noexcept { return ColourRGB( Zero, Max, Max ); }
	static constexpr ColourRGB Magenta() noexcept { return ColourRGB( Max, Zero, Max ); }

	static constexpr ColourRGB Orange() noexcept { return ColourRGB( Max, Max / 2, Zero ); }
	static constexpr ColourRGB Violet() noexcept { return ColourRGB( Max / 2, Zero, Max ); }
	static constexpr ColourRGB Purple() noexcept { return ColourRGB( Max / 2, Zero, Max / 2 ); }

	// member data

	T r;
	T g;
	T b;
};

template <typename T>
struct ColourRGBA
{
	static_assert( std::is_arithmetic_v<T>, "T must be a numeric type" );

public:
	using value_type = T;
	using size_type = std::size_t;

	static constexpr T Max = std::is_floating_point_v<T> ? static_cast<T>( 1 ) : std::numeric_limits<T>::max();
	static constexpr T Zero = static_cast<T>( 0 );

	// construction

	ColourRGBA() noexcept = default;

	constexpr ColourRGBA( T red, T green, T blue, T alpha = Max ) noexcept
		: r{ red } , g{ green } , b{ blue }, a{ alpha }
	{}

	constexpr ColourRGBA( T rgb, T alpha ) noexcept
		: r{ rgb } , g{ rgb } , b{ rgb } , a{ alpha }
	{}

	constexpr explicit ColourRGBA( T rgb ) noexcept
		: r{ rgb } , g{ rgb } , b{ rgb } , a{ Max }
	{}

	constexpr ColourRGBA( const ColourRGBA& ) noexcept = default;

	constexpr ColourRGBA( const ColourRGB<T>& other, T alpha ) noexcept
		: r{ other.r } , g{ other.g } , b{ other.b } , a{ alpha }
	{}

	constexpr ColourRGBA( const ColourRGB<T>& other ) noexcept
		: r{ other.r } , g{ other.g } , b{ other.b } , a{ Max }
	{}

	// access

	constexpr T& operator[]( size_type index ) noexcept
	{
		dbExpects( index < 4 );
		return ( &r )[ index ];
	}

	constexpr const T& operator[]( size_type index ) const noexcept
	{
		dbExpects( index < 4 );
		return ( &r )[ index ];
	}

	// comparison

	friend constexpr bool operator==( const ColourRGBA& lhs, const ColourRGBA& rhs ) noexcept
	{
		return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
	}

	friend constexpr bool operator!=( const ColourRGBA& lhs, const ColourRGBA& rhs ) noexcept
	{
		return !( lhs == rhs );
	}

	// basic colours

	static constexpr ColourRGBA Black() noexcept { return ColourRGBA( Zero ); }
	static constexpr ColourRGBA White() noexcept { return ColourRGBA( Max ); }
	
	static constexpr ColourRGBA DarkGrey() noexcept { return ColourRGBA( Max / 4 ); }
	static constexpr ColourRGBA Grey() noexcept { return ColourRGBA( Max / 2 ); }
	static constexpr ColourRGBA LightGrey() noexcept { return ColourRGBA( ( Max / 4 ) * 3 ); }

	static constexpr ColourRGBA Red() noexcept { return ColourRGBA( Max, Zero, Zero ); }
	static constexpr ColourRGBA Green() noexcept { return ColourRGBA( Zero, Max, Zero ); }
	static constexpr ColourRGBA Blue() noexcept { return ColourRGBA( Zero, Zero, Max ); }

	static constexpr ColourRGBA Yellow() noexcept { return ColourRGBA( Max, Max, Zero ); }
	static constexpr ColourRGBA Cyan() noexcept { return ColourRGBA( Zero, Max, Max ); }
	static constexpr ColourRGBA Magenta() noexcept { return ColourRGBA( Max, Zero, Max ); }

	static constexpr ColourRGBA Orange() noexcept { return ColourRGBA( Max, Max / 2, Zero ); }
	static constexpr ColourRGBA Violet() noexcept { return ColourRGBA( Max / 2, Zero, Max ); }
	static constexpr ColourRGBA Purple() noexcept { return ColourRGBA( Max / 2, Zero, Max / 2 ); }

	// member data

	T r;
	T g;
	T b;
	T a;
};

template <typename T>
constexpr ColourRGB<T> FromRGBCode( uint32_t rgb ) noexcept
{
	constexpr T Ratio = ColourRGB<T>::Max / static_cast<T>( 255 );

	return ColourRGB(
		static_cast<T>( rgb >> 16 ) * Ratio,
		static_cast<T>( rgb >> 8 ) * Ratio,
		static_cast<T>( rgb ) * Ratio
	);
}

template <typename T>
constexpr ColourRGBA<T> FromRGBACode( uint32_t rgba ) noexcept
{
	constexpr T Ratio = ColourRGBA<T>::Max / static_cast<T>( 255 );

	return ColourRGBA(
		static_cast<T>( rgba >> 24 ) * Ratio,
		static_cast<T>( rgba >> 16 ) * Ratio,
		static_cast<T>( rgba >> 8 ) * Ratio,
		static_cast<T>( rgba ) * Ratio
	);
}

template <typename T>
constexpr uint32_t ToRGBCode( const ColourRGB<T>& c ) noexcept
{
	constexpr T Max = ColourRGB<T>::Max;
	constexpr T Zero = ColourRGB<T>::Zero;
	constexpr T InvRatio = Max / static_cast<T>( 255 );

	return ( static_cast<uint8_t>( std::clamp( c.r, Zero, Max ) / InvRatio ) << 16 ) |
		( static_cast<uint8_t>( std::clamp( c.g, Zero, Max ) / InvRatio ) << 8 ) |
		( static_cast<uint8_t>( std::clamp( c.b, Zero, Max ) / InvRatio ) );
}

template <typename T>
constexpr uint32_t ToRGBACode( const ColourRGBA<T>& c ) noexcept
{
	constexpr T Max = ColourRGBA<T>::Max;
	constexpr T Zero = ColourRGBA<T>::Zero;
	constexpr T InvRatio = Max / static_cast<T>( 255 );

	return ( static_cast<uint8_t>( std::clamp( c.r, Zero, Max ) / InvRatio ) << 24 ) |
		( static_cast<uint8_t>( std::clamp( c.g, Zero, Max ) / InvRatio ) << 16 ) |
		( static_cast<uint8_t>( std::clamp( c.b, Zero, Max ) / InvRatio ) << 8 ) |
		static_cast<uint8_t>( std::clamp( c.a, Zero, Max ) / InvRatio );
}

template <typename To, typename From>
constexpr ColourRGB<To> ColourCast( const ColourRGB<From>& from ) noexcept
{
	if constexpr ( std::is_integral_v<To> &&
		std::is_integral_v<From> &&
		static_cast<std::make_unsigned_t<To>>( ColourRGB<To>::Max ) < static_cast<std::make_unsigned_t<From>>( ColourRGB<From>::Max ) )
	{
		constexpr auto InvRatio = ColourRGB<From>::Max / ColourRGB<To>::Max;

		return ColourRGB{
			static_cast<To>( from.r / InvRatio ),
			static_cast<To>( from.g / InvRatio ),
			static_cast<To>( from.b / InvRatio ),
			static_cast<To>( from.a / InvRatio ) };
	}
	else
	{
		constexpr auto Ratio = ColourRGB<To>::Max / ColourRGB<From>::Max;

		return ColourRGB{
			static_cast<To>( from.r * Ratio ),
			static_cast<To>( from.g * Ratio ),
			static_cast<To>( from.b * Ratio ),
			static_cast<To>( from.a * Ratio ) };
	}
}

template <typename To, typename From>
constexpr ColourRGBA<To> ColourCast( const ColourRGBA<From>& from ) noexcept
{
	if constexpr ( std::is_integral_v<To> &&
		std::is_integral_v<From> &&
		static_cast<std::make_unsigned_t<To>>( ColourRGBA<To>::Max ) < static_cast<std::make_unsigned_t<From>>( ColourRGBA<From>::Max ) )
	{
		constexpr auto InvRatio = ColourRGBA<From>::Max / ColourRGBA<To>::Max;

		return ColourRGBA{
			static_cast<To>( from.r / InvRatio ),
			static_cast<To>( from.g / InvRatio ),
			static_cast<To>( from.b / InvRatio ),
			static_cast<To>( from.a / InvRatio ) };
	}
	else
	{
		constexpr auto Ratio = ColourRGBA<To>::Max / ColourRGBA<From>::Max;

		return ColourRGBA{
			static_cast<To>( from.r * Ratio ),
			static_cast<To>( from.g * Ratio ),
			static_cast<To>( from.b * Ratio ),
			static_cast<To>( from.a * Ratio ) };
	}
}

template <typename T>
constexpr ColourRGB<T> Invert( const ColourRGB<T>& c ) noexcept
{
	constexpr T Max = ColourRGB<T>::Max;
	return ColourRGB{ Max - c.r, Max - c.g, Max - c.b };
}

template <typename T>
constexpr ColourRGBA<T> Invert( const ColourRGBA<T>& c ) noexcept
{
	constexpr T Max = ColourRGBA<T>::Max;
	return ColourRGBA{ Max - c.r, Max - c.g, Max - c.b, c.a };
}

template <typename T>
constexpr T Intensity( const ColourRGB<T>& c ) noexcept
{
	return static_cast<T>( c.r * 0.3 + c.g * 0.59 + c.b * 0.11 );
}

template <typename T>
constexpr T Intensity( const ColourRGBA<T>& c ) noexcept
{
	return static_cast<T>( c.r * 0.3 + c.g * 0.59 + c.b * 0.11 );
}

template <typename T>
constexpr ColourRGB<T> GreyScale( const ColourRGB<T>& c ) noexcept
{
	const T grey = Intensity( c );
	return ColourRGBA<T>{ grey, grey, grey };
}

template <typename T>
constexpr ColourRGBA<T> GreyScale( const ColourRGBA<T>& c ) noexcept
{
	const T grey = Intensity( c );
	return ColourRGBA<T>{ grey, grey, grey, c.a };
}

template <typename T>
constexpr ColourRGB<T> Sepia( const ColourRGB<T>& c ) noexcept
{
	// sepia multipiers recommended by Microsoft

	constexpr double Max = static_cast<double>( ColourRGB<T>::Max );

	return ColourRGB<T>
	{
		static_cast<T>( ( std::min )( c.r * 0.393 + c.g * 0.769 + c.b * 0.189, Max ) ),
		static_cast<T>( ( std::min )( c.r * 0.349 + c.g * 0.686 + c.b * 0.168, Max ) ),
		static_cast<T>( ( std::min )( c.r * 0.272 + c.g * 0.534 + c.b * 0.131, Max ) )
	};
}

template <typename T>
constexpr ColourRGBA<T> Sepia( const ColourRGBA<T>& c ) noexcept
{
	return ColourRGBA<T>{ Sepia( static_cast<const ColourRGB<T>&>( c ) ), c.a };
}

template <typename T>
constexpr ColourRGB<T> Blend( const ColourRGB<T>& dest, const ColourRGB<T>& src, T srcAlpha ) noexcept
{
	constexpr T Max = ColourRGB<T>::Max;

	const T inv_src_a = Max - srcAlpha;

	return ColourRGB
	{
		static_cast<T>( ( src.r * srcAlpha + dest.r * inv_src_a ) / Max ),
		static_cast<T>( ( src.g * srcAlpha + dest.g * inv_src_a ) / Max ),
		static_cast<T>( ( src.b * srcAlpha + dest.b * inv_src_a ) / Max )
	};
}

template <typename T>
constexpr ColourRGBA<T> Blend( const ColourRGBA<T>& dest, const ColourRGBA<T>& src ) noexcept
{
	constexpr T Max = ColourRGBA<T>::Max;

	const T inv_src_a = Max - src.a;

	return ColourRGBA
	{
		static_cast<T>( ( src.r * src.a + ( dest.r * inv_src_a * dest.a ) / Max ) / Max ),
		static_cast<T>( ( src.g * src.a + ( dest.g * inv_src_a * dest.a ) / Max ) / Max ),
		static_cast<T>( ( src.b * src.a + ( dest.b * inv_src_a * dest.a ) / Max ) / Max ),
		static_cast<T>( src.a + ( inv_src_a * dest.a ) / Max )
	};
}

using ColourRGB8 = ColourRGB<uint8_t>;
using ColourRGBA8 = ColourRGBA<uint8_t>;
using ColourRGB16 = ColourRGB<uint16_t>;
using ColourRGBA16 = ColourRGBA<uint16_t>;
using ColourRGBf = ColourRGB<float>;
using ColourRGBAf = ColourRGBA<float>;
using ColourRGBd = ColourRGB<double>;
using ColourRGBAd = ColourRGBA<double>;

}

namespace std
{
	template<typename T>
	constexpr Math::ColourRGB<T> max( const Math::ColourRGB<T>& lhs, const Math::ColourRGB<T>& rhs ) noexcept
	{
		return Math::ColourRGB<T>(
			( std::max )( lhs.r, rhs.r ),
			( std::max )( lhs.g, rhs.g ),
			( std::max )( lhs.b, rhs.b ) );
	}

	template<typename T>
	constexpr Math::ColourRGBA<T> max( const Math::ColourRGBA<T>& lhs, const Math::ColourRGBA<T>& rhs ) noexcept
	{
		return Math::ColourRGBA<T>(
			( std::max )( lhs.r, rhs.r ),
			( std::max )( lhs.g, rhs.g ),
			( std::max )( lhs.b, rhs.b ),
			( std::max )( lhs.a, rhs.a ) );
	}

	template<typename T>
	constexpr Math::ColourRGB<T> min( const Math::ColourRGB<T>& lhs, const Math::ColourRGB<T>& rhs ) noexcept
	{
		return Math::ColourRGB<T>(
			( std::min )( lhs.r, rhs.r ),
			( std::min )( lhs.g, rhs.g ),
			( std::min )( lhs.b, rhs.b ) );
	}

	template<typename T>
	constexpr Math::ColourRGBA<T> min( const Math::ColourRGBA<T>& lhs, const Math::ColourRGBA<T>& rhs ) noexcept
	{
		return Math::ColourRGBA<T>(
			( std::min )( lhs.r, rhs.r ),
			( std::min )( lhs.g, rhs.g ),
			( std::min )( lhs.b, rhs.b ),
			( std::min )( lhs.a, rhs.a ) );
	}
}

static_assert( Math::Blend( Math::ColourRGB8::White(), Math::ColourRGB8::White(), (uint8_t)255 ) == Math::ColourRGB8::White() );