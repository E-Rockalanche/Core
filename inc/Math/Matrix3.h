#pragma once

#include <stdx/assert.h>
#include <Math/Vector3.h>

#include <array>

namespace Math
{

template <typename T>
struct Matrix3
{
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;

	constexpr Vector3f& operator[]( size_t index ) noexcept
	{
		return rows[ index ];
	}

	constexpr const Vector3f& operator[]( size_t index ) const noexcept
	{
		return rows[ index ];
	}

	constexpr Matrix3& operator+=( const Matrix3& other ) noexcept
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	constexpr Matrix3& operator-=( const Matrix3& other ) noexcept
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	constexpr Matrix3& operator*=( T value ) noexcept
	{
		for ( auto& element : elements )
		{
			element *= value;
		}
		return *this;
	}

	constexpr Matrix3& operator/=( T value ) noexcept
	{
		dbExpects( value != 0 );
		for ( auto& element : elements )
		{
			element /= value;
		}
		return *this;
	}

	constexpr T Determinant() const noexcept
	{
		return x[ 0 ] * ( y[ 1 ] * z[ 2 ] - y[ 2 ] * z[ 1 ] ) +
			x[ 1 ] * ( y[ 2 ] * z[ 0 ] - y[ 0 ] * z[ 2 ] ) +
			x[ 2 ] * ( y[ 0 ] * z[ 1 ] - y[ 1 ] * z[ 0 ] );
	}

	constexpr Vector3f<T> Scale() const noexcept
	{
		return Vector3f{ x.Magnitude(), y.Magnitude(), z.Magnitude() };
	}

	constexpr Matrix3& Transpose() noexcept
	{
		for ( size_t j = 0; j < Height; ++j )
		{
			for ( size_t i = j + 1; i < Width; ++i )
			{
				std::swap( m_matrix[ j ][ i ], m_matrix[ i ][ j ] );
			}
		}
		return *this;
	}

	constexpr Matrix3& Orthonormalize() noexcept
	{
		// Gram-Schmidt process
		// https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process

		y -= Project( y, x );
		z -= Project( z, x ) + Project( z, y );

		if ( Determinant() != 0 )
		{
			x /= x.Magnitude();
			y /= y.Magnitude();
			z /= z.Magnitude();
		}
		else
		{
			*this = Matrix3::Zero();
		}

		return *this;
	}

	static constexpr Matrix3 Zero() noexcept
	{
		return Matrix3{ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
	}

	static constexpr Matrix3 One() noexcept
	{
		return Matrix3{ { 1, 1, 1, 1, 1, 1, 1, 1, 1 } };
	}

	static constexpr Matrix3 Identity() noexcept
	{
		return Matrix3{ { 1, 0, 0, 0, 1, 0, 0, 0, 1 } };
	}

	union
	{
		std::array<T, 9> elements;
		std::array<Vector3<T>, 3> rows;
		struct
		{
			Vector3<T> x;
			Vector3<T> y;
			Vector3<T> z;
		};
	};

	static_assert( sizeof( Matrix3 ) == sizeof( T ) * 9 );
};

template <typename T>
constexpr Matrix3<T> operator+( Matrix3<T> lhs, const Matrix3<T>& rhs ) noexcept
{
	return lhs += rhs;
}

template <typename T>
constexpr Matrix3<T> operator-( Matrix3<T> lhs, const Matrix3<T>& rhs ) noexcept
{
	return lhs -= rhs;
}

template <typename T>
constexpr Matrix3<T> operator*( Matrix3<T> m, T value ) noexcept
{
	return m *= value;
}

template <typename T>
constexpr Matrix3<T> operator*( T value, Matrix3<T> m ) noexcept
{
	return m *= value;
}

template <typename T>
constexpr Matrix3<T> operator/( Matrix3<T> m, T value ) noexcept
{
	return m /= value;
}

template <typename T>
constexpr Matrix3<T> operator*( const Matrix3<T>& lhs, const Matrix3<T>& rhs ) noexcept
{
	Matrix3<T> result = Matrix3<T>::Zero();

	for ( size_t j = 0; j < 3; ++j )
	{
		for ( size_t i = 0; i < 3; ++i )
		{
			T sum = 0;
			for ( size_t k = 0; k < 3; ++k )
			{
				sum += lhs[ j ][ k ] * rhs[ k ][ i ];
			}
			result[ j ][ i ] = sum;
		}
	}

	return result;
}

template <typename T>
constexpr Vector3<T> operator*( const Matrix3<T>& lhs, const Vector3<T>& rhs ) noexcept
{
	Vector3<T> result{ 0 };

	for ( size_t j = 0; j < 3; ++j )
	{
		T sum = 0;
		for ( size_t k = 0; k < 3; ++k )
		{
			sum += lhs[ j ][ k ] * rhs[ k ];
		}
		result[ j ] = sum;
	}

	return result;
}

}