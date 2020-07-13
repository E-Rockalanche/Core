#pragma once

#include <stdx/assert.h>
#include <stdx/utility.h>

#include <cmath>

namespace Math
{

template <typename T> struct Vector3;
template <typename T> struct Position3;
template <typename T> struct Normal3;

template <typename T>
struct Vector3
{
	using value_type = T;

	Vector3() noexcept = default;

	constexpr Vector3( const Vector3& ) noexcept = default;

	constexpr Vector3( T x_, T y_, T z_ ) noexcept : x{ x_ }, y{ y_ }, z{ z_ } {}

	constexpr Vector3( T xyz ) noexcept : x{ xyz }, y{ xyz }, z{ xyz } {}

	constexpr T& operator[]( size_t index ) noexcept
	{
		dbExpects( index < 3 );
		return ( &x )[ index ];
	}

	constexpr const T& operator[]( size_t index ) const noexcept
	{
		dbExpects( index < 3 );
		return ( &x )[ index ];
	}

	constexpr Vector3 operator+=( const Vector3& other ) noexcept
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	constexpr Vector3 operator-=( const Vector3& other ) noexcept
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	constexpr Vector3 operator*=( const Vector3& other ) noexcept
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}

	constexpr Vector3 operator/=( const Vector3& other ) noexcept
	{
		dbExpects( other.x != 0 );
		dbExpects( other.y != 0 );
		dbExpects( other.z != 0 );
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}

	constexpr Vector3 operator*=( T s ) noexcept
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	constexpr Vector3 operator/=( T s ) noexcept
	{
		dbExpects( s != 0 );
		x /= s;
		y /= s;
		z /= s;
		return *this;
	}

	constexpr T Magnitude() const noexcept
	{
		return std::sqrt( SqrMagnitude() );
	}

	constexpr T SqrMagnitude() const noexcept
	{
		return x * x + y * y + z * z;
	}

	T x;
	T y;
	T z;
};

template <typename T>
constexpr bool operator==( const Vector3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	return ( lhs.x == rhs.x ) && ( lhs.y == rhs.y ) && ( lhs.z == rhs.z );
}

template <typename T>
constexpr bool operator!=( const Vector3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
constexpr Vector3<T> operator+( Vector3<T> lhs, const Vector3<T>& rhs ) noexcept
{
	return lhs += rhs;
}

template <typename T>
constexpr Vector3<T> operator-( Vector3<T> lhs, const Vector3<T>& rhs ) noexcept
{
	return lhs -= rhs;
}

template <typename T>
constexpr Vector3<T> operator*( Vector3<T> lhs, const Vector3<T>& rhs ) noexcept
{
	return lhs *= rhs;
}

template <typename T>
constexpr Vector3<T> operator/( Vector3<T> lhs, const Vector3<T>& rhs ) noexcept
{
	return lhs /= rhs;
}

template <typename T>
constexpr Vector3<T> operator*( Vector3<T> v, T s ) noexcept
{
	return v *= s;
}

template <typename T>
constexpr Vector3<T> operator*( T s, Vector3<T> v ) noexcept
{
	return v *= s;
}

template <typename T>
constexpr Vector3<T> operator/( Vector3<T> v, T s ) noexcept
{
	return lhs /= s;
}

template <typename T>
constexpr T DotProduct( const Vector3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	return ( lhs.x * rhs.x ) + ( lhs.y * rhs.y ) + ( lhs.z * rhs.z );
}

template <typename T>
constexpr Vector3<T> CrossProduct( const Vector3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	return Vector3<T>
	{
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x
	};
}

template <typename T>
struct Position3
{
	using value_type = T;

	constexpr Position3() noexcept = default;

	constexpr Position3( const Position3& ) noexcept = default;

	constexpr Position3( T x_, T y_, T z_ ) noexcept : x{ x_ }, y{ y_ }, z{ z_ } {}

	constexpr T& operator[]( size_t index ) noexcept
	{
		dbExpects( index < 3 );
		return ( &x )[ index ];
	}

	constexpr const T& operator[]( size_t index ) const noexcept
	{
		dbExpects( index < 3 );
		return ( &x )[ index ];
	}

	constexpr Position3& operator+=( const Vector3& v ) noexcept
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	constexpr Position3& operator-=( const Vector3& v ) noexcept
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	T x = 0;
	T y = 0;
};

template <typename T>
constexpr bool operator==( const Position3<T>& lhs, const Position3<T>& rhs ) noexcept
{
	return ( lhs.x == rhs.x ) && ( lhs.y == rhs.y ) && ( lhs.z == rhs.z );
}

template <typename T>
constexpr bool operator!=( const Position3<T>& lhs, const Position3<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
constexpr Position3<T> operator+( Position3<T> p, const Vector3<T>& v ) noexcept
{
	return p += v;
}

template <typename T>
constexpr Position3<T> operator+( const Vector3<T>& v, Position3<T> p ) noexcept
{
	return p += v;
}

template <typename T>
constexpr Position3<T> operator-( Position3<T> p, const Vector3<T>& v ) noexcept
{
	return p -= v;
}

template <typename T>
constexpr Vector3<T> operator-( const Position3<T>& lhs, const Position3<T>& rhs ) noexcept
{
	return Vector3<T>{ lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

template <typename T>
struct Normal3
{
	constexpr Normal3() noexcept = default;

	constexpr Normal3( const Normal& ) noexcept = default;

	constexpr Normal3( T x_, T y_, T z_ ) noexcept
	{
		auto length = std::hypot( x_, y_, z_ );
		dbExpects( length > 0 );
		m_x = x_ / length;
		m_y = y_ / length;
		m_z = z_ / length;
	}

	constexpr x() const noexcept { return m_x; }
	constexpr y() const noexcept { return m_y; }
	constexpr z() const noexcept { return m_z; }

private:
	T m_x = 1;
	T m_y = 0;
	T m_z = 0;
};

template <typename T>
constexpr bool operator==( const Normal3<T>& lhs, const Normal3<T>& rhs ) noexcept
{
	return ( lhs.x() == rhs.x() ) && ( lhs.y() == rhs.y() ) && ( lhs.z() == rhs.z() );
}

template <typename T>
constexpr bool operator!=( const Normal3<T>& lhs, const Normal3<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
constexpr Vector3<T> operator*( const Normal3<T>& n, T s ) noexcept
{
	return Vector3<T>{ n.x() * s, n.y() * s, n.z() * s };
}

template <typename T>
constexpr Vector3<T> operator/( const Normal3<T>& n, T s ) noexcept
{
	dbExpects( s != 0 );
	return Vector3<T>{ n.x() / s, n.y() / s, n.z() / s };
}

template <typename T>
constexpr Normal3<T> Normalize( const Vector3<T>& v ) noexcept
{
	return Normal3( v.x, v.y, v.z );
}

template <typename T>
constexpr T DotProduct( const Normal3<T>& lhs, const Normal3<T>& rhs ) noexcept
{
	return ( lhs.x() * rhs.x() ) + ( lhs.y() * rhs.y() ) + ( lhs.z() * rhs.z() );
}

template <typename T>
constexpr T DotProduct( const Vector3<T>& lhs, const Normal3<T>& rhs ) noexcept
{
	return ( lhs.x * rhs.x() ) + ( lhs.y * rhs.y() ) + ( lhs.z * rhs.z() );
}

template <typename T>
constexpr T DotProduct( const Normal3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	return ( lhs.x() * rhs.x ) + ( lhs.y() * rhs.y ) + ( lhs.z() * rhs.z );
}

template <typename T>
constexpr Vector3<T> Project( const Vector3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	return rhs * ( DotProduct( lhs, rhs ) / rhs.SqrMagnitude() );
}

template <typename T>
constexpr Vector3<T> Project( const Vector3<T>& lhs, const Normal3<T>& rhs ) noexcept
{
	return rhs * DotProduct( lhs, rhs );
}

template <typename T>
constexpr T AngleBetween( const Vector3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	return std::acos( DotProduct( lhs, rhs ) );
}

template <typename T>
constexpr T AngleBetween( const Normal3<T>& lhs, const Normal3<T>& rhs ) noexcept
{
	return std::acos( DotProduct( lhs, rhs ) );
}

using Vector3f = Vector3<float>;
using Position3f = Position3<float>;
using Normal3f = Normal3<float>;

} // namespace Math

namespace std
{

template<typename T>
struct hash<Math::Vector3<T>>
{
	std::size_t operator()( const Math::Vector3<T>& value ) const noexcept
	{
		auto h = std::hash<T>{}( value.x );
		stdx::hash_combine( h, value.y );
		stdx::hash_combine( h, value.z );
		return h;
	}
};

template<typename T>
struct hash<Math::Position3<T>>
{
	std::size_t operator()( const Math::Position3<T>& value ) const noexcept
	{
		auto h = std::hash<T>{}( value.x );
		stdx::hash_combine( h, value.y );
		stdx::hash_combine( h, value.z );
		return h;
	}
};

template<typename T>
struct hash<Math::Normal3<T>>
{
	std::size_t operator()( const Math::Normal3<T>& value ) const noexcept
	{
		auto h = std::hash<T>{}( value.x() );
		stdx::hash_combine( h, value.y() );
		stdx::hash_combine( h, value.z() );
		return h;
	}
};

}