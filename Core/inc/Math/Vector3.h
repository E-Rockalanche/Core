#pragma once

#include <stdx/assert.h>

#include <cmath>

namespace Math
{

template <typename T> struct Vector3;
template <typename T> struct Position3;
template <typename T> class Normal3;

template <typename T>
struct Vector3
{
	using value_type = T;
	using size_type = std::size_t;

	Vector3() noexcept = default;

	constexpr Vector3( const Vector3& ) noexcept = default;

	constexpr explicit Vector3( const Position3<T>& position ) noexcept;

	constexpr explicit Vector3( const Normal3<T>& position ) noexcept;

	constexpr Vector3( T x_, T y_, T z_ ) noexcept : x{ x_ }, y{ y_ }, z{ z_ } {}

	constexpr explicit Vector3( T xyz ) noexcept : x{ xyz }, y{ xyz }, z{ xyz } {}

	constexpr T& operator[]( size_type index ) noexcept
	{
		dbExpects( index < 3 );
		return ( &x )[ index ];
	}

	constexpr const T& operator[]( size_type index ) const noexcept
	{
		dbExpects( index < 3 );
		return ( &x )[ index ];
	}

	constexpr Vector3& operator+=( const Vector3& other ) noexcept
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	constexpr Vector3& operator-=( const Vector3& other ) noexcept
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	constexpr Vector3& operator*=( const Vector3& other ) noexcept
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}

	constexpr Vector3& operator/=( const Vector3& other ) noexcept
	{
		dbExpects( other.x != 0 );
		dbExpects( other.y != 0 );
		dbExpects( other.z != 0 );
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}

	constexpr Vector3& operator*=( T s ) noexcept
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	constexpr Vector3& operator/=( T s ) noexcept
	{
		dbExpects( s != 0 );
		x /= s;
		y /= s;
		z /= s;
		return *this;
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
constexpr Vector3<T> operator-( const Vector3<T>& v ) noexcept
{
	return { -v.x, -v.y, -v.z };
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
	return v /= s;
}

template <typename T>
struct Position3
{
	using value_type = T;
	using size_type = std::size_t;

	constexpr Position3() noexcept = default;

	constexpr Position3( const Position3& ) noexcept = default;

	constexpr explicit Position3( const Vector3<T>& vec ) noexcept
		: x{ vec.x }, y{ vec.y }, z{ vec.z }
	{}

	constexpr Position3( T x_, T y_, T z_ ) noexcept : x{ x_ }, y{ y_ }, z{ z_ } {}

	constexpr explicit Position3( T xyz ) noexcept : x{ xyz }, y{ xyz }, z{ xyz } {}

	constexpr T& operator[]( size_type index ) noexcept
	{
		dbExpects( index < 3 );
		return ( &x )[ index ];
	}

	constexpr const T& operator[]( size_type index ) const noexcept
	{
		dbExpects( index < 3 );
		return ( &x )[ index ];
	}

	constexpr Position3& operator+=( const Vector3<T>& v ) noexcept
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	constexpr Position3& operator-=( const Vector3<T>& v ) noexcept
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	T x = 0;
	T y = 0;
	T z = 0;
};

template <typename T>
constexpr Vector3<T>::Vector3( const Position3<T>& pos ) noexcept
	: x{ pos.x }, y{ pos.y }, z{ pos.z }
{}

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
class Normal3
{
public:
	using size_type = std::size_t;

	struct ScaleTag {};
	struct NoScaleTag {};

	Normal3() noexcept = default; // cannot be constexpr. Starts in invalid state

	constexpr Normal3( const Normal3& ) noexcept = default;

	constexpr Normal3( ScaleTag, T x, T y, T z ) noexcept
	{
		auto length = std::hypot( x, y, z );
		dbExpects( length > 0 );
		m_x = x / length;
		m_y = y / length;
		m_z = z / length;
	}

	constexpr Normal3( NoScaleTag, T x, T y, T z ) noexcept
		: m_x{ x }, m_y{ y }, m_z{ z }
	{
		dbExpects( std::hypot( x, y, z ) == 1 );
	}

	constexpr const T& operator[]( size_type index ) const noexcept
	{
		dbExpects( index < 3 );
		return ( &m_x )[ index ];
	}

	constexpr const T& x() const noexcept { return m_x; }
	constexpr const T& y() const noexcept { return m_y; }
	constexpr const T& z() const noexcept { return m_z; }

private:
	T m_x = 0;
	T m_y = 0;
	T m_z = 0;
};

template <typename T>
constexpr Vector3<T>::Vector3( const Normal3<T>& pos ) noexcept
	: x{ pos.x() }, y{ pos.y() }, z{ pos.z() }
{}

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
constexpr Normal3<T> operator-( const Normal3<T>& n ) noexcept
{
	return Normal3<T>( Normal3<T>::NoScaleTag{}, -n.x(), -n.y(), -n.z() );
}

template <typename T>
constexpr Vector3<T> operator*( const Normal3<T>& n, T s ) noexcept
{
	return Vector3<T>{ n.x() * s, n.y() * s, n.z() * s };
}

template <typename T>
constexpr Vector3<T> operator*( T s, const Normal3<T>& n ) noexcept
{
	return Vector3<T>{ n.x() * s, n.y() * s, n.z() * s };
}

template <typename T>
constexpr Vector3<T> operator/( const Normal3<T>& n, T s ) noexcept
{
	dbExpects( s != 0 );
	return Vector3<T>{ n.x() / s, n.y() / s, n.z() / s };
}

// get magnitude of vector

template <typename T>
constexpr T Magnitude( const Vector3<T>& v ) noexcept
{
	return static_cast<T>( std::hypot( v.x, v.y, v.z ) );
}

template <typename T>
constexpr T Magnitude( const Normal3<T>& ) noexcept
{
	return T{ 1 };
}

template <typename T>
constexpr T SqrMagnitude( const Vector3<T>& v ) noexcept
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

template <typename T>
constexpr T SqrMagnitude( const Normal3<T>& ) noexcept
{
	return T{ 1 };
}

// get normal of vector

template <typename T>
constexpr Normal3<T> Normalize( const Vector3<T>& v ) noexcept
{
	return Normal3<T>( Normal3<T>::ScaleTag{}, v.x, v.y, v.z );
}

// dot product of vectors

template <typename T>
constexpr T DotProduct( const Vector3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	return ( lhs.x * rhs.x ) + ( lhs.y * rhs.y ) + ( lhs.z * rhs.z );
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

// corss product of vectors

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
constexpr Vector3<T> CrossProduct( const Normal3<T>& lhs, const Normal3<T>& rhs ) noexcept
{
	return Vector3<T>
	{
		lhs.y() * rhs.z() - lhs.z() * rhs.y(),
		lhs.z() * rhs.x() - lhs.x() * rhs.z(),
		lhs.x() * rhs.y() - lhs.y() * rhs.x()
	};
}

template <typename T>
constexpr Vector3<T> CrossProduct( const Vector3<T>& lhs, const Normal3<T>& rhs ) noexcept
{
	return Vector3<T>
	{
		lhs.y * rhs.z() - lhs.z * rhs.y(),
		lhs.z * rhs.x() - lhs.x * rhs.z(),
		lhs.x * rhs.y() - lhs.y * rhs.x()
	};
}

template <typename T>
constexpr Vector3<T> CrossProduct( const Normal3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	return Vector3<T>
	{
		lhs.y() * rhs.z - lhs.z() * rhs.y,
		lhs.z() * rhs.x - lhs.x() * rhs.z,
		lhs.x() * rhs.y - lhs.y() * rhs.x
	};
}

// project vector onto another

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

// angle between vectors

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

using Vector3d = Vector3<double>;
using Position3d = Position3<double>;
using Normal3d = Normal3<double>;

} // namespace Math