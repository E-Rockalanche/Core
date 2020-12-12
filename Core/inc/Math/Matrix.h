#pragma once

#include <stdx/assert.h>

#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>

namespace Math
{

// templated matrix type with row major layout
template <size_t Height, size_t Width, typename T>
struct Matrix
{
public:
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using iterator = typename std::array<T, Height * Width>::iterator;
	using const_iterator = typename std::array<T, Height * Width>::const_iterator;

	static constexpr Matrix Zero() noexcept;
	static constexpr Matrix Identity() noexcept;

	static constexpr size_t width = Width;
	static constexpr size_t height = Height;
	static constexpr size_t area = Width * Height;

	struct FillTag {};

	// construction

	Matrix() noexcept = default;

	constexpr Matrix( const Matrix& ) = default;

	explicit constexpr Matrix( const T& value ) : elements{}
	{
		static_assert( Height == Width, "only square matrices can construct with diagonal" );

		for ( size_t i = 0; i < Height; ++i )
			rows[ i ][ i ] = value;
	}

	constexpr Matrix( FillTag, const T& value ) : Matrix( value, std::make_index_sequence<area>() ) {}

	constexpr Matrix( std::initializer_list<T> init ) : Matrix( init, std::make_index_sequence<area>() ) {}

	constexpr Matrix& operator=( const Matrix& other ) noexcept
	{
		elements = other.elements;
		return *this;
	}

	// modifiers

	constexpr Matrix& operator+=( const Matrix& rhs ) noexcept
	{
		std::transform( begin(), end(), rhs.begin(), begin(), std::plus{} );
	}

	constexpr Matrix& operator-=( const Matrix& rhs ) noexcept
	{
		std::transform( begin(), end(), rhs.begin(), begin(), std::minus{} );
	}

	constexpr Matrix& operator*=( T s ) noexcept
	{
		for ( auto& value : elements )
		{
			value *= s;
		}
	}

	constexpr Matrix& operator/=( T s ) noexcept
	{
		dbExpects( s != 0 );
		for ( auto& value : elements )
		{
			value /= s;
		}
	}

	constexpr void Transpose() noexcept
	{
		static_assert( Height == Width, "only square matrices can self transpose" );

		for ( size_t j = 0; j < Height; ++j )
		{
			for ( size_t i = j + 1; i < Width; ++i )
			{
				std::swap( rows[ j ][ i ], rows[ i ][ j ] );
			}
		}
	}

	// access

	constexpr auto operator[]( size_type row ) noexcept { return rows[ row ].begin(); }
	constexpr auto operator[]( size_type row ) const noexcept { return rows[ row ].begin(); }

	constexpr reference get( size_t j, size_t i ) noexcept { return rows[ j ][ i ]; }
	constexpr const_reference get( size_t j, size_t i ) const noexcept { return rows[ j ][ i ]; }

	constexpr pointer data() noexcept { return elements.data(); }
	constexpr const_pointer data() const noexcept { return elements.data(); }

	constexpr size_type size() const noexcept { return area; }
	constexpr difference_type ssize() const noexcept { return static_cast<difference_type>( area ); }

	constexpr iterator begin() noexcept { return elements.begin(); }
	constexpr iterator end() noexcept { return elements.begin() + area; }

	constexpr const_iterator begin() const noexcept { return elements.begin(); }
	constexpr const_iterator end() const noexcept { return elements.begin() + area; }

	constexpr const_iterator cbegin() const noexcept { return elements.begin(); }
	constexpr const_iterator cend() const noexcept { return elements.begin() + area; }

	// non member functions

	friend constexpr Matrix operator+( Matrix lhs, const Matrix& rhs ) noexcept
	{
		return lhs += rhs;
	}

	friend constexpr Matrix operator-( Matrix lhs, const Matrix& rhs ) noexcept
	{
		return lhs -= rhs;
	}

	friend constexpr Matrix operator*( Matrix lhs, T rhs ) noexcept
	{
		return lhs *= rhs;
	}

	friend constexpr Matrix operator*( T lhs, Matrix rhs ) noexcept
	{
		return rhs *= lhs;
	}

	friend constexpr Matrix operator/( Matrix lhs, T rhs ) noexcept
	{
		return lhs /= rhs;
	}

	union
	{
		std::array<T, area> elements;
		std::array<std::array<T, Width>, Height> rows;
	};

private:
	template <size_t>
	static constexpr const T& make_value_for_index( const T& value ) noexcept { return value; }

	template <size_t... Is>
	constexpr Matrix( const T& value, std::index_sequence<Is...> indicies )
		: elements{ make_value_for_index<Is>( value )... }
	{}

	template <size_t... Is>
	constexpr Matrix( std::initializer_list<T> init, std::index_sequence<Is...> indicies )
		: elements{ *( init.begin() + Is )... }
	{
		dbExpects( init.size() == area );
	}
};

template <size_t H, size_t W, typename T>
constexpr Matrix<H, W, T> Matrix<H, W, T>::Zero() noexcept
{
	return Matrix( FillTag{}, 0 );
}

template <size_t H, size_t W, typename T>
constexpr Matrix<H, W, T> Matrix<H, W, T>::Identity() noexcept
{
	static_assert( H == W, "Only square matrices can have an identity" );

	return Matrix( 1 );
}

template <size_t H1, size_t WH, size_t W2, typename T>
constexpr Matrix<H1, W2, T> operator*( const Matrix<H1, WH, T>& lhs, const Matrix<WH, W2, T>& rhs ) noexcept
{
	Matrix<H1, W2, T> result( 0 );
	for ( size_t j = 0; j < H1; ++j )
	{
		for ( size_t i = 0; i < W2; ++i )
		{
			auto& sum = result[ j ][ i ];
			for ( size_t k = 0; k < WH; ++k )
			{
				sum += lhs[ j ][ k ] * rhs[ k ][ i ];
			}
		}
	}
	return result;
}

template <size_t H, size_t W, typename T>
constexpr Matrix<W, H, T> Transpose( const Matrix<H, W, T>& other ) noexcept
{
	Matrix<W, H, T> result( 0 );
	for ( size_t j = 0; j < H; ++j )
	{
		for ( size_t i = 0; i < W; ++i )
		{
			result[ i ][ j ] = other[ j ][ i ];
		}
	}
	return result;
}

template <size_t H, size_t W, typename T>
constexpr T Determinant( const Matrix<H, W, T>& m ) noexcept
{
	static_assert( H == W, "only square matrices have a determinant" );

	if constexpr ( H == 1 ) // fast 1x1
	{
		return m[ 0 ][ 0 ];
	}
	else if constexpr ( H == 2 ) // fast 2x2
	{
		return m[ 0 ][ 0 ] * m[ 1 ][ 1 ] - m[ 0 ][ 1 ] * m[ 1 ][ 0 ];
	}
	else if constexpr ( H == 3 ) // fast 3x3
	{
		return ( m[ 0 ][ 0 ] * ( m[ 1 ][ 1 ] * m[ 2 ][ 2 ] - m[ 1 ][ 2 ] * m[ 2 ][ 1 ] ) ) -
			( m[ 1 ][ 0 ] * ( m[ 0 ][ 1 ] * m[ 2 ][ 2 ] - m[ 0 ][ 2 ] * m[ 2 ][ 1 ] ) ) +
			( m[ 2 ][ 0 ] * ( m[ 0 ][ 1 ] * m[ 1 ][ 2 ] - m[ 0 ][ 2 ] * m[ 1 ][ 1 ] ) );
	}
	else // any size
	{
		T result = 0;
		for ( size_t k = 0; k < H; ++k )
		{
			// build submatrix
			Matrix<T, H - 1, W - 1> subMatrix( 0 );
			size_t subJ = 0;
			for ( size_t j = 0; j < H; ++j )
			{
				if ( j != k )
				{
					std::copy( m[ j ] + 1, m[ j ] + W, subMatrix[ subJ ] + 1 );
					++subJ;
				}
			}

			result += ( k == 0 ? 1 : -1 ) * m[ k ][ 0 ] * Determinant( subMatrix );
		}
		return result;
	}
}

template <size_t H, size_t W, typename T>
Matrix<H, W, T> Inverse( const Matrix<H, W, T>& matrix ) noexcept
{
	static_assert( H == W, "only square matrices have an inverse" );

	constexpr size_t W2 = W * 2;

	// initialize intermediate matrix with original on left and identity on right
	Matrix<T, H, W2> intermediate;
	for ( size_t j = 0; j < H; ++j )
	{
		std::copy( matrix[ j ], matrix[ j ] + W, intermediate[ j ] );
		std::fill( intermediate[ j ] + W, intermediate[ j ] + W2, 0 );
		intermediate[ j ][ j + W ] = 1;
	}

	for ( size_t i = 0; i < W; ++i )
	{
		// find non-zero leading row
		T lead = intermediate[ i ][ i ];
		if ( lead == 0 )
		{
			for ( size_t j = i + 1; j < H; ++j )
			{
				if ( intermediate[ j ][ i ] != 0 )
				{
					// found. Swap rows i and j
					std::swap( intermediate[ i ] + i, intermediate[ i ] + W2, intermediate[ j ] + i );
					lead = intermediate[ i ][ i ];
					break;
				}
			}

			if ( lead == 0 )
				continue; // column i is all zeros
		}
		// else row i already has non-zero lead

		// cancel out column i
		for ( size_t j = i + 1; j < H; ++j )
		{
			const T multiplier = -intermediate[ j ][ i ] / lead; // multiplies row i before adding to row j
			intermediate[ j ][ i ] = 0;

			T* rowj = intermediate[ j ] + i + 1;
			std::transform( intermediate[ i ] + i + 1, intermediate[ i ] + W2, rowj, rowj, [multiplier]( T lhs, T rhs ) { return lhs * multiplier + rhs; } );
		}
	}

	// copt transformed identity into result matrix
	Matrix<H, W, T> result;
	for ( size_t j = 0; j < H; ++j )
	{
		std::copy( intermediate[ j ] + W, intermediate[ j ] + W2, result[ j ] );
	}
	return result;
}

using Matrix2f = Matrix<2, 2, float>;
using Matrix3f = Matrix<3, 3, float>;
using Matrix4f = Matrix<4, 4, float>;

} // namespace Math