#pragma once

#include <stdx/assert.h>
#include <cstdint>

namespace Math
{

class Colour
{
public:
	constexpr Colour() = default;

	constexpr Colour( float red, float green, float blue, float alpha = 1.0f )
		: r{ red }
		, g{ green }
		, b{ blue }
		, a{ alpha }
	{}

	constexpr Colour( float rgb, float a = 1.0f ) : Colour( rgb, rgb, rgb, a ) {}

	constexpr Colour( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255 )
		: Colour( red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f )
	{}

	constexpr Colour( uint8_t rgb, uint8_t a = 255 ) : Colour( rgb, rgb, rgb, a ) {}

	constexpr Colour( uint32_t rgba ) : Colour(
		static_cast<uint8_t>( rgba >> 24 ),
		static_cast<uint8_t>( rgba >> 16 ),
		static_cast<uint8_t>( rgba >> 8 ),
		static_cast<uint8_t>( rgba ) )
	{}

	constexpr float& operator[]( size_t index ) noexcept
	{
		dbExpects( index < 4 );
		return ( &r )[ index ];
	}

	constexpr float operator[]( size_t index ) const noexcept
	{
		dbExpects( index < 4 );
		return ( &r )[ index ];
	}

	constexpr operator uint32_t() const noexcept
	{
		const uint8_t red = static_cast<uint8_t>( std::clamp<float>( r, 0.0f, 1.0f ) * 255 );
		const uint8_t green = static_cast<uint8_t>( std::clamp<float>( g, 0.0f, 1.0f ) * 255 );
		const uint8_t blue = static_cast<uint8_t>( std::clamp<float>( b, 0.0f, 1.0f ) * 255 );
		const uint8_t alpha = static_cast<uint8_t>( std::clamp<float>( a, 0.0f, 1.0f ) * 255 );

		return ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
	}

	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;
};

}