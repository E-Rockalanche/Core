#pragma once

#include <stdx/assert.h>

#include <cmath>

namespace Math
{

template <typename T> struct Vector4;
template <typename T> struct Position4;
template <typename T> class Normal4;

template <typename T>
struct Vector4
{
	using value_type = T;
	using size_type = std::size_t;

	Vector4() noexcept = default;

	constexpr Vector4( const Vector4& ) noexcept = default;

	constexpr explicit Vector4( const Position4<T>& position ) noexcept;

	constexpr explicit Vector4( const Normal4<T>& position ) noexcept;

	constexpr Vector4( T x_, T y_, T z_, T w_ ) noexcept : x{ x_ }, y{ y_ }, z{ z_ }, w{ w_ } {}

	constexpr explicit Vector4( T xyzw ) noexcept : x{ xyz }, y{ xyz }, z{ xyz } w{ xyzw } {}

	constexpr T& operator[]( size_type index ) noexcept
	{
		dbExpects( index < 4 );
		return ( &x )[ index ];
	}

	constexpr const T& operator[]( size_type index ) const noexcept
	{
		dbExpects( index < 4 );
		return ( &x )[ index ];
	}

	constexpr Vector4& operator+=( const Vector4& other ) noexcept
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	constexpr Vector4& operator-=( const Vector4& other ) noexcept
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	constexpr Vector4& operator*=( const Vector4& other ) noexcept
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w
		return *this;
	}

	constexpr Vector4& operator/=( const Vector4& other ) noexcept
	{
		dbExpects( other.x != 0 );
		dbExpects( other.y != 0 );
		dbExpects( other.z != 0 );
		dbExpects( other.w != 0 );
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
	}

	constexpr Vector4& operator*=( T s ) noexcept
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return *this;
	}

	constexpr Vector4& operator/=( T s ) noexcept
	{
		dbExpects( s != 0 );
		x /= s;
		y /= s;
		z /= s;
		w /= s;
		return *this;
	}

	T x;
	T y;
	T z;
	T w;
};

template <typename T>
constexpr bool operator==( const Vector4<T>& lhs, const Vector4<T>& rhs ) noexcept
{
	return ( lhs.x == rhs.x ) && ( lhs.y == rhs.y ) && ( lhs.z == rhs.z ) && ( lhs.w == rhs.w );
}

template <typename T>
constexpr bool operator!=( const Vector4<T>& lhs, const Vector4<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
constexpr Vector4<T> operator+( Vector4<T> lhs, const Vector4<T>& rhs ) noexcept
{
	return lhs += rhs;
}

template <typename T>
constexpr Vector4<T> operator-( Vector4<T> lhs, const Vector4<T>& rhs ) noexcept
{
	return lhs -= rhs;
}

template <typename T>
constexpr Vector4<T> operator-( const Vector4<T>& v ) noexcept
{
	return { -v.x, -v.y, -v.z, -v.w };
}

template <typename T>
constexpr Vector4<T> operator*( Vector4<T> lhs, const Vector4<T>& rhs ) noexcept
{
	return lhs *= rhs;
}

template <typename T>
constexpr Vector4<T> operator/( Vector4<T> lhs, const Vector4<T>& rhs ) noexcept
{
	return lhs /= rhs;
}

template <typename T>
constexpr Vector4<T> operator*( Vector4<T> v, T s ) noexcept
{
	return v *= s;
}

template <typename T>
constexpr Vector4<T> operator*( T s, Vector4<T> v ) noexcept
{
	return v *= s;
}

template <typename T>
constexpr Vector4<T> operator/( Vector4<T> v, T s ) noexcept
{
	return v /= s;
}

template <typename T>
struct Position4
{
	using value_type = T;
	using size_type = std::size_t;

	Position4() noexcept = default;

	constexpr Position4( const Position4& ) noexcept = default;

	constexpr explicit Position4( const Vector4<T>& vec ) noexcept
		: x{ vec.x }, y{ vec.y }, z{ vec.z }, w{ vec.w }
	{}

	constexpr Position4( T x_, T y_, T z_, T w_ ) noexcept : x{ x_ }, y{ y_ }, z{ z_ }, w{ w_ } {}

	constexpr explicit Position4( T xyzw ) noexcept : x{ xyz }, y{ xyz }, z{ xyz }, w{ xyzw } {}

	constexpr T& operator[]( size_type index ) noexcept
	{
		dbExpects( index < 4 );
		return ( &x )[ index ];
	}

	constexpr const T& operator[]( size_type index ) const noexcept
	{
		dbExpects( index < 4 );
		return ( &x )[ index ];
	}

	constexpr Position4& operator+=( const Vector4<T>& v ) noexcept
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	constexpr Position4& operator-=( const Vector4<T>& v ) noexcept
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	T x;
	T y;
	T z;
	T w;
};

template <typename T>
constexpr Vector4<T>::Vector4( const Position4<T>& pos ) noexcept
	: x{ pos.x }, y{ pos.y }, z{ pos.z }, w{ pos.x }
{}

template <typename T>
constexpr bool operator==( const Position4<T>& lhs, const Position4<T>& rhs ) noexcept
{
	return ( lhs.x == rhs.x ) && ( lhs.y == rhs.y ) && ( lhs.z == rhs.z ) && ( lhs.w == rhs.w );
}

template <typename T>
constexpr bool operator!=( const Position4<T>& lhs, const Position4<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
constexpr Position4<T> operator+( Position4<T> p, const Vector4<T>& v ) noexcept
{
	return p += v;
}

template <typename T>
constexpr Position4<T> operator+( const Vector4<T>& v, Position4<T> p ) noexcept
{
	return p += v;
}

template <typename T>
constexpr Position4<T> operator-( Position4<T> p, const Vector4<T>& v ) noexcept
{
	return p -= v;
}

template <typename T>
constexpr Vector4<T> operator-( const Position4<T>& lhs, const Position4<T>& rhs ) noexcept
{
	return Vector4<T>{ lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w };
}

template <typename T>
class Normal4
{
public:
	using size_type = std::size_t;

	struct ScaleTag {};
	struct NoScaleTag {};

	Normal4() noexcept = default;

	constexpr Normal4( const Normal4& ) noexcept = default;

	constexpr Normal4( ScaleTag, T x, T y, T z, T w ) noexcept
	{
		auto length = std::hypot( x, y, z, w );
		dbExpects( length > 0 );
		m_x = x / length;
		m_y = y / length;
		m_z = z / length;
		m_w = w / length;
	}

	constexpr Normal4( NoScaleTag, T x, T y, T z, T w ) noexcept
		: m_x{ x }, m_y{ y }, m_z{ z }, m_w{ w }
	{
		dbExpects( std::hypot( x, y, z, w ) == 1 );
	}

	constexpr const T& operator[]( size_type index ) const noexcept
	{
		dbExpects( index < 4 );
		return ( &m_x )[ index ];
	}

	constexpr const T& x() const noexcept { return m_x; }
	constexpr const T& y() const noexcept { return m_y; }
	constexpr const T& z() const noexcept { return m_z; }
	constexpr const T& w() const noexcept { return m_w; }

private:
	T m_x;
	T m_y;
	T m_z;
	T m_w;
};

template <typename T>
constexpr Vector4<T>::Vector4( const Normal4<T>& n ) noexcept
	: x{ n.x() }, y{ n.y() }, z{ n.z() }, w{ n.w }
{}

template <typename T>
constexpr bool operator==( const Normal4<T>& lhs, const Normal4<T>& rhs ) noexcept
{
	return ( lhs.x() == rhs.x() ) && ( lhs.y() == rhs.y() ) && ( lhs.z() == rhs.z() ) && ( lhs.w() == rhs.w() );
}

template <typename T>
constexpr bool operator!=( const Normal4<T>& lhs, const Normal4<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

template <typename T>
constexpr Normal4<T> operator-( const Normal4<T>& n ) noexcept
{
	return Normal4<T>( Normal4<T>::NoScaleTag{}, -n.x(), -n.y(), -n.z(), -n.w() );
}

template <typename T>
constexpr Vector4<T> operator*( const Normal4<T>& n, T s ) noexcept
{
	return Vector4<T>{ n.x() * s, n.y() * s, n.z() * s, n.w() * s };
}

template <typename T>
constexpr Vector4<T> operator*( T s, const Normal4<T>& n ) noexcept
{
	return Vector4<T>{ n.x() * s, n.y() * s, n.z() * s, n.w() * s };
}

template <typename T>
constexpr Vector4<T> operator/( const Normal4<T>& n, T s ) noexcept
{
	dbExpects( s != 0 );
	return Vector4<T>{ n.x() / s, n.y() / s, n.z() / s, n.w() / s };
}

// get magnitude of vector

template <typename T>
constexpr T Magnitude( const Vector4<T>& v ) noexcept
{
	return static_cast<T>( std::hypot( v.x, v.y, v.z, v.w ) );
}

template <typename T>
constexpr T Magnitude( const Normal4<T>& ) noexcept
{
	return T{ 1 };
}

template <typename T>
constexpr T SqrMagnitude( const Vector4<T>& v ) noexcept
{
	return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

template <typename T>
constexpr T SqrMagnitude( const Normal4<T>& ) noexcept
{
	return T{ 1 };
}

// get normal of vector

template <typename T>
constexpr Normal4<T> Normalize( const Vector4<T>& v ) noexcept
{
	return Normal4<T>( Normal4<T>::ScaleTag{}, v.x, v.y, v.z, v.w );
}

// dot product of vectors

template <typename T>
constexpr T DotProduct( const Vector4<T>& lhs, const Vector4<T>& rhs ) noexcept
{
	return ( lhs.x * rhs.x ) + ( lhs.y * rhs.y ) + ( lhs.z * rhs.z ) + ( lhs.w * rhs.w );
}

template <typename T>
constexpr T DotProduct( const Normal4<T>& lhs, const Normal4<T>& rhs ) noexcept
{
	return ( lhs.x() * rhs.x() ) + ( lhs.y() * rhs.y() ) + ( lhs.z() * rhs.z() ) + ( lhs.w() * rhs.w() );
}

template <typename T>
constexpr T DotProduct( const Vector4<T>& lhs, const Normal4<T>& rhs ) noexcept
{
	return ( lhs.x * rhs.x() ) + ( lhs.y * rhs.y() ) + ( lhs.z * rhs.z() ) + ( lhs.w * rhs.w() );
}

template <typename T>
constexpr T DotProduct( const Normal4<T>& lhs, const Vector4<T>& rhs ) noexcept
{
	return ( lhs.x() * rhs.x ) + ( lhs.y() * rhs.y ) + ( lhs.z() * rhs.z ) + ( lhs.w() * rhs.w );
}

// project vector onto another

template <typename T>
constexpr Vector4<T> Project( const Vector4<T>& lhs, const Vector4<T>& rhs ) noexcept
{
	return rhs * ( DotProduct( lhs, rhs ) / rhs.SqrMagnitude() );
}

template <typename T>
constexpr Vector4<T> Project( const Vector4<T>& lhs, const Normal4<T>& rhs ) noexcept
{
	return rhs * DotProduct( lhs, rhs );
}

// angle between vectors

template <typename T>
constexpr T AngleBetween( const Vector4<T>& lhs, const Vector4<T>& rhs ) noexcept
{
	return std::acos( DotProduct( lhs, rhs ) );
}

template <typename T>
constexpr T AngleBetween( const Normal4<T>& lhs, const Normal4<T>& rhs ) noexcept
{
	return std::acos( DotProduct( lhs, rhs ) );
}

using Vector4f = Vector4<float>;
using Position4f = Position4<float>;
using Normal4f = Normal4<float>;

using Vector4d = Vector4<double>;
using Position4d = Position4<double>;
using Normal4d = Normal4<double>;

} // namespace Math