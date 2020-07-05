#pragma once

#include <stdx/assert.h>

#include <limits>
#include <type_traits>
#include <utility>

namespace Math
{

template <typename T>
class Colour
{
	static_assert( std::is_arithmetic_v<T>, "T must be a numeric type" );

public:
	using value_type = T;
	using index_type = std::size_t;

	static constexpr T Max = std::is_floating_point_v<T> ? static_cast<T>( 1 ) : std::numeric_limits<T>::max();
	static constexpr T Zero = static_cast<T>( 0 );

	// construction

	Colour() noexcept = default;

	constexpr Colour( T red, T green, T blue, T alpha = Max ) noexcept
		: r{ red }
		, g{ green }
		, b{ blue }
		, a{ alpha }
	{}

	constexpr Colour( T rgb, T alpha = Max ) noexcept
		: r{ rgb }
		, g{ rgb }
		, b{ rgb }
		, a{ alpha }
	{}

	constexpr Colour( const Colour& ) noexcept = default;

	// access

	constexpr T& operator[]( index_type index ) noexcept
	{
		dbExpects( index < 4 );
		return ( &r )[ index ];
	}

	constexpr const T& operator[]( index_type index ) const noexcept
	{
		dbExpects( index < 4 );
		return ( &r )[ index ];
	}

	// comparison

	friend constexpr bool operator==( const Colour& lhs, const Colour& rhs ) noexcept
	{
		return lhs.r == rhs.r &&
			lhs.g == rhs.g &&
			lhs.b == rhs.b &&
			lhs.a == rhs.a;
	}

	friend constexpr bool operator!=( const Colour& lhs, const Colour& rhs ) noexcept
	{
		return !( lhs == rhs );
	}

	// basic colours

	static constexpr Colour Black() noexcept { return Colour( Zero ); }
	static constexpr Colour White() noexcept { return Colour( Max ); }
	
	static constexpr Colour DarkGrey() noexcept { return Colour( Max / 4 ); }
	static constexpr Colour Grey() noexcept { return Colour( Max / 2 ); }
	static constexpr Colour LightGrey() noexcept { return Colour( ( Max / 4 ) * 3 ); }

	static constexpr Colour Red() noexcept { return Colour( Max, Zero, Zero ); }
	static constexpr Colour Green() noexcept { return Colour( Zero, Max, Zero ); }
	static constexpr Colour Blue() noexcept { return Colour( Zero, Zero, Max ); }

	static constexpr Colour Yellow() noexcept { return Colour( Max, Max, Zero ); }
	static constexpr Colour Cyan() noexcept { return Colour( Zero, Max, Max ); }
	static constexpr Colour Magenta() noexcept { return Colour( Max, Zero, Max ); }

	static constexpr Colour Orange() noexcept { return Colour( Max, Max / 2, Zero ); }
	static constexpr Colour Violet() noexcept { return Colour( Max / 2, Zero, Max ); }
	static constexpr Colour Purple() noexcept { return Colour( Max / 2, Zero, Max / 2 ); }

	// member data

	T r;
	T g;
	T b;
	T a;
};

template <typename T>
constexpr Colour<T> ColourFromRGBACode( uint32_t rgba ) noexcept
{
	constexpr T Ratio = Colour<T>::Max / static_cast<T>( 255 );

	return Colour(
		static_cast<T>( rgba >> 24 ) * Ratio,
		static_cast<T>( rgba >> 16 ) * Ratio,
		static_cast<T>( rgba >> 8 ) * Ratio,
		static_cast<T>( rgba ) * Ratio
	);
}

template <typename T>
constexpr uint32_t ColourToRGBACode( const Colour<T>& c ) noexcept
{
	constexpr T Max = Colour<T>::Max;
	constexpr T Zero = Colour<T>::Zero;
	constexpr T InvRatio = Max / static_cast<T>( 255 );

	return ( static_cast<uint8_t>( std::clamp( c.r, Zero, Max ) / InvRatio ) << 24 ) |
		( static_cast<uint8_t>( std::clamp( c.g, Zero, Max ) / InvRatio ) << 16 ) |
		( static_cast<uint8_t>( std::clamp( c.b, Zero, Max ) / InvRatio ) << 8 ) |
		static_cast<uint8_t>( std::clamp( c.a, Zero, Max ) / InvRatio );
}

template <typename To, typename From>
constexpr Colour<To> ColourCast( const Colour<From>& from ) noexcept
{
	if constexpr ( std::is_integral_v<To> &&
		std::is_integral_v<From> &&
		static_cast<std::make_unsigned_t<To>>( Colour<To>::Max ) < static_cast<std::make_unsigned_t<From>>( Colour<From>::Max ) )
	{
		constexpr auto InvRatio = Colour<From>::Max / Colour<To>::Max;

		return Colour{
			static_cast<To>( from.r / InvRatio ),
			static_cast<To>( from.g / InvRatio ),
			static_cast<To>( from.b / InvRatio ),
			static_cast<To>( from.a / InvRatio ) };
	}
	else
	{
		constexpr auto Ratio = Colour<To>::Max / Colour<From>::Max;

		return Colour{
			static_cast<To>( from.r * Ratio ),
			static_cast<To>( from.g * Ratio ),
			static_cast<To>( from.b * Ratio ),
			static_cast<To>( from.a * Ratio ) };
	}
}

template <typename T>
constexpr Colour<T> ColourInvert( const Colour<T>& c ) noexcept
{
	constexpr T Max = Colour<T>::Max;
	return Colour{ Max - c.r, Max - c.g, Max - c.b, c.a };
}

template <typename T>
constexpr T ColourIntensity( const Colour<T>& c ) noexcept
{
	return static_cast<T>( ( c.r * 0.3 + c.g * 0.59 + c.b * 0.11 ) * c.a / Colour<T>::Max );
}

template <typename T>
constexpr Colour<T> ColourGreyScale( const Colour<T>& c ) noexcept
{
	const T grey = Intensity( c );
	return Colour<T>{ grey, grey, grey, c.a };
}

template <typename T>
constexpr Colour<T> ColourSepia( const Colour<T>& c ) noexcept
{
	// sepia multipiers recommended by Microsoft

	constexpr double Max = static_cast<double>( Colour<T>::Max );

	return Colour<T>
	{
		static_cast<T>( (std::min)( c.r * 0.393 + c.g * 0.769 + c.b * 0.189, Max ) ),
		static_cast<T>( (std::min)( c.r * 0.349 + c.g * 0.686 + c.b * 0.168, Max ) ),
		static_cast<T>( (std::min)( c.r * 0.272 + c.g * 0.534 + c.b * 0.131, Max ) ),
		c.a
	};
}

template <typename T>
constexpr Colour<T> ColourBlend( const Colour<T>& dest, const Colour<T>& src ) noexcept
{
	constexpr T Max = Colour<T>::Max;

	const T inv_src_a = src.a - Max;

	return Colour
	{
		static_cast<T>( ( src.r * src.a ) / Max + ( dest.r * inv_src_a * dest.a ) / ( Max * Max ) ),
		static_cast<T>( ( src.g * src.a ) / Max + ( dest.g * inv_src_a * dest.a ) / ( Max * Max ) ),
		static_cast<T>( ( src.b * src.a ) / Max + ( dest.b * inv_src_a * dest.a ) / ( Max * Max ) ),
		static_cast<T>( src.a + ( inv_src_a * dest.a ) / Max )
	};
}

namespace std
{

	template<typename T>
	Colour<T> max( const Colour<T>& lhs, const Colour<T>& rhs ) noexcept
	{
		return Colour<T>(
			( std::max )( lhs.r, rhs.r ),
			( std::max )( lhs.g, rhs.g ),
			( std::max )( lhs.b, rhs.b ),
			( std::max )( lhs.a, rhs.a ) );
	}

	template<typename T>
	Colour<T> min( const Colour<T>& lhs, const Colour<T>& rhs ) noexcept
	{
		return Colour<T>(
			( std::min )( lhs.r, rhs.r ),
			( std::min )( lhs.g, rhs.g ),
			( std::min )( lhs.b, rhs.b ),
			( std::min )( lhs.a, rhs.a ) );
	}

}

using Colourf = Colour<float>;
using Colour32 = Colour<uint8_t>;

}