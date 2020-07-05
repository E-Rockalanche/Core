#pragma once

#include <stdx/assert.h>
#include <cmath>

namespace Math
{

template <typename T>
struct Vector2
{
public:
	using value_type = T;

	constexpr Vector2() noexcept = default;
	constexpr Vector2( const Vector2& ) noexcept = default;
	constexpr Vector2( T x_, T y_ ) noexcept : x{ x_ }, y{ y_ } {}

	constexpr T& operator[]( size_t index ) noexcept
	{
		dbAssert( index < 2 );
		return ( &x )[ index ];
	}

	constexpr const T& operator[]( size_t index ) const noexcept
	{
		dbAssert( index < 2 );
		return ( &x )[ index ];
	}

	constexpr Vector2& operator+=( const Vector2& rhs ) noexcept
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	constexpr Vector2& operator-=( const Vector2& rhs ) noexcept
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	template <typename U>
	constexpr Vector2& operator*=( const U& value ) noexcept
	{
		x = static_cast<T>( x * value );
		y = static_cast<T>( y * value );
		return *this;
	}

	template <typename U>
	constexpr Vector2& operator/=( const U& value ) noexcept
	{
		dbExpects( value != 0 );
		x = static_cast<T>( x / value );
		y = static_cast<T>( y / value );
		return *this;
	}

	float Length() const noexcept
	{
		return std::hypot( static_cast<float>( x ), static_cast<float>( y ) );
	}

	static constexpr T DotProduct( const Vector2& lhs, const Vector2& rhs ) noexcept
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}

	T x = 0;
	T y = 0;
};

template <typename T>
constexpr bool operator==( const Vector2<T>& lhs, const Vector2<T>& rhs ) noexcept
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <typename T>
constexpr bool operator!=( const Vector2<T>& lhs, const Vector2<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
constexpr Vector2<T> operator+( Vector2<T> lhs, const Vector2<T>& rhs ) noexcept
{
	return lhs += rhs;
}

template <typename T>
constexpr Vector2<T> operator-( Vector2<T> lhs, const Vector2<T>& rhs ) noexcept
{
	return lhs -= rhs;
}

template <typename T, typename U>
constexpr Vector2<T> operator*( Vector2<T> lhs, const U& rhs ) noexcept
{
	return lhs *= rhs;
}

template <typename T, typename U>
constexpr Vector2<T> operator*( const U& lhs, Vector2<T> rhs ) noexcept
{
	return rhs *= lhs;
}

template <typename T, typename U>
constexpr Vector2<T> operator/( Vector2<T> lhs, const U& rhs ) noexcept
{
	return lhs /= rhs;
}

template <typename T>
struct Position2
{
public:
	using value_type = T;

	constexpr Position2() noexcept = default;
	constexpr Position2( const Position2& ) noexcept = default;
	constexpr Position2( T x_, T y_ ) noexcept : x{ x_ }, y{ y_ } {}

	constexpr T& operator[]( size_t index ) noexcept
	{
		dbAssert( index < 2 );
		return ( &x )[ index ];
	}

	constexpr const T& operator[]( size_t index ) const noexcept
	{
		dbAssert( index < 2 );
		return ( &x )[ index ];
	}

	constexpr Position2& operator+=( const Vector2<T>& rhs ) noexcept
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	constexpr Position2& operator-=( const Vector2<T>& rhs ) noexcept
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	T x = 0;
	T y = 0;
};

template <typename T>
constexpr bool operator==( const Position2<T>& lhs, const Position2<T>& rhs ) noexcept
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <typename T>
constexpr bool operator!=( const Position2<T>& lhs, const Position2<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
constexpr Position2<T> operator+( Position2<T> lhs, const Vector2<T>& rhs ) noexcept
{
	return lhs += rhs;
}

template <typename T>
constexpr Position2<T> operator-( Position2<T> lhs, const Vector2<T>& rhs ) noexcept
{
	return lhs -= rhs;
}

template <typename T>
constexpr Vector2<T> operator-( const Position2<T>& lhs, const Position2<T>& rhs ) noexcept
{
	return Vector2<T>{ lhs.x - rhs.x, lhs.y - rhs.y };
}

using Vector2f = Vector2<float>;
using Position2f = Position2<float>;

using Vector2i = Vector2<int>;
using Position2i = Position2<int>;

} // namespace Math