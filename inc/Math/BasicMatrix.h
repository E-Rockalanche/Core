#pragma once

#include <stdx/assert.h>

#include <type_traits>
#include <utility>

namespace Math
{

template <size_t Height, size_t Width, typename T>
class BasicMatrix
{
public:
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;

	using iterator = pointer;
	using const_iterator = const_pointer;

	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	// construction

	BasicMatrix() noexcept = default;

	constexpr BasicMatrix( const BasicMatrix& ) = default;

	constexpr BasicMatrix( const T& value ) : BasicMatrix( value, std::make_integer_sequence<Height * Width>() ) {}

	constexpr BasicMatrix( T m[ Height ][ Width ] ) : m_matrix{ m } {}

	constexpr BasicMatrix( T d[ Height * Width ] ) : m_data{ d } {}

	// operations

	template <size_t H1, size_t WH, size_t W2>
	friend BasicMatrix<H1, W2> operator*( const BasicMatrix<H1, WH>& lhs, const BasicMatrix<WH, W2>& rhs );

	BasicMatrix& operator+=( const BasicMatrix& rhs ) noexcept
	{
		auto dest = begin();
		auto end = end();
		auto src = rhs.begin();
		for ( ; dest != end; ++dest, ++src )
		{
			*dest += *src;
		}
	}

	BasicMatrix& operator-=( const BasicMatrix& rhs ) noexcept
	{
		auto dest = begin();
		auto end = end();
		auto src = rhs.begin();
		for ( ; dest != end; ++dest, ++src )
		{
			*dest -= *src;
		}
	}

	BasicMatrix& operator*=( T scalar ) noexcept
	{
		for ( auto& value : *this )
		{
			value *= scalar;
		}
	}

	BasicMatrix& operator/=( T scalar ) noexcept
	{
		dbAssert( scalar != 0 );
		for ( auto& value : *this )
		{
			value /= scalar;
		}
	}

	friend BasicMatrix operator+( BasicMatrix lhs, const BasicMatrix& rhs ) noexcept
	{
		return lhs += rhs;
	}

	friend BasicMatrix operator-( BasicMatrix lhs, const BasicMatrix& rhs ) noexcept
	{
		return lhs -= rhs;
	}

	template <typename S>
	friend BasicMatrix operator*( BasicMatrix lhs, const S& rhs ) noexcept
	{
		return lhs *= rhs;
	}

	template <typename S>
	friend BasicMatrix operator*( const S& lhs, BasicMatrix rhs ) noexcept
	{
		return rhs *= lhs;
	}

	template <typename S>
	friend BasicMatrix operator/( BasicMatrix lhs, const S& rhs ) noexcept
	{
		return lhs /= rhs;
	}

	constexpr void Transpose() noexcept
	{
		for ( size_t j = 0; j < Height; ++j )
		{
			for ( size_t i = j + 1; i < Width; ++i )
			{
				std::swap( m_matrix[ j ][ i ], m_matrix[ i ][ j ] );
			}
		}
	}

	static constexpr BasicMatrix Transpose( const BasicMatrix& rhs ) noexcept
	{
		BasicMatrix result( rhs );
		result.Transpose();
		return result;
	}

	// access

	constexpr pointer data() noexcept
	{
		return &m_data[ 0 ][ 0 ];
	}

	constexpr const_pointer data() const noexcept
	{
		return &m_data[ 0 ][ 0 ];
	}

	constexpr size_type width() const noexcept { return Width; }
	constexpr size_type height() const noexcept { return Height; }

	constexpr size_type size() const noexcept { return Width * Height; }
	constexpr difference_type ssize() const noexcept { return static_cast<difference_type>( Width * Height ); }

	constexpr iterator begin() noexcept { return data(); }
	constexpr iterator end() noexcept { return data() + size(); }

	constexpr const_iterator begin() const noexcept { return data(); }
	constexpr const_iterator end() const noexcept { return data() + size(); }

	constexpr const_iterator cbegin() const noexcept { return begin(); }
	constexpr const_iterator cend() const noexcept { return end(); }

	// presets

	static constexpr BasicMatrix Zero() noexcept;

	static constexpr BasicMatrix One() noexcept;

	// template <std::enable_if_t<H == W, int> = 0>
	static constexpr BasicMatrix Identity() noexcept;

private:

	template <int>
	static constexpr const T& make_value_for_index( const T& value ) noexcept { return value; }

	template <int... Is>
	constexpr BasicMatrix( const T& value, std::integer_sequence<int, Is...> indicies ) : m_data{ make_value_for_index<Is>( value )... } {}

private:
	union
	{
		T m_matrix[ Height ][ Width ];
		T m_data[ Height * Width ];
	}
};


template <size_t H1, size_t WH, size_t W2, typename T>
constexpr BasicMatrix<H1, W2, T> operator*( const BasicMatrix<H1, WH, T>& lhs, const BasicMatrix<WH, W2, T>& rhs ) noexcept
{
	BasicMatrix<H1, W2, T> result( 0 );
	for ( size_t j = 0; j < H1; ++j )
	{
		for ( size_t i = 0; i < W2; ++i )
		{
			auto& sum = result[ j ][ i ];
			sum = 0;
			for ( size_t k = 0; k < WH; ++k )
			{
				sum += lhs[ j ][ k ] * rhs[ k ][ i ];
			}
		}
	}
	return result;
}

template <size_t H, size_t W, typename T>
constexpr BasicMatrix<H, W, T> BasicMatrix<H, W, T>::Zero() noexcept
{
	return BasicMatrix( 0 );
}

template <size_t H, size_t W, typename T>
constexpr BasicMatrix<H, W, T> BasicMatrix<H, W, T>::One() noexcept
{
	return BasicMatrix( 1 );
}

template <size_t H, size_t W, typename T>
constexpr BasicMatrix<H, W, T> BasicMatrix<H, W, T>::Identity() noexcept
{
	static_assert( H == W, "Only square matrices can have an identity" );

	auto matrix = BasicMatrix<H, W>::Zero();
	for ( size_t i = 0; i < H; ++i )
	{
		matrix[ i ][ i ] = 1;
	}
	return matrix;
}

} // namespace Math