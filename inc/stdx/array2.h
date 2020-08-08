#pragma once

#include <stdx/memory.h>

#include <stdx/int.h>

#include <algorithm>
#include <iterator>

namespace stdx
{

template <typename T>
class array2
{
public:
	using element_type = T;
	using value_type = std::remove_cv_t<T>;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using difference_type = std::ptrdiff_t;
	using index_type = std::ptrdiff_t;
	using size_type = std::size_t;

	// construction/assignment

	array2() noexcept = default;

	array2( index_type w, index_type h )
	{
		defaultAllocate( stdx::narrow_cast<size_type>( w ), stdx::narrow_cast<size_type>( h ) );
	}

	array2( index_type w, index_type h, const T& value )
	{
		overwriteAllocate( stdx::narrow_cast<size_type>( w ), stdx::narrow_cast<size_type>( h ) );
		fill( value );
	}

	array2( array2&& ) noexcept = default;

	array2( const array2& other )
	{
		overwriteAllocate( other.m_width, other.m_height );
		std::copy( other.begin(), other.end(), begin() );
	}

	array2& operator=( array2&& other ) noexcept = default;

	array2& operator=( const array2& other )
	{
		if ( m_width != other.m_width && m_height != other.m_height )
			overwriteAllocate( other.m_width, other.m_height );

		std::copy( other.begin(), other.end(), begin() );
		return *this;
	}

	// access

	pointer data() noexcept { return m_data.get(); }
	const_pointer data() const noexcept { return m_data.get(); }

	reference get( index_type x, index_type y ) noexcept
	{
		return m_data[ get_pos( x, y ) ];
	}

	const_reference get( index_type x, index_type y ) const noexcept
	{
		return m_data[ get_pos( x, y ) ];
	}

	// iterators

	iterator begin() noexcept { return m_data.get(); }
	iterator end() noexcept { return begin() + size(); }

	const_iterator begin() const noexcept { return m_data.get(); }
	const_iterator end() const noexcept { return begin() + size(); }

	const_iterator cbegin() const noexcept { return begin(); }
	const_iterator cend() const noexcept { return end(); }

	reverse_iterator rbegin() noexcept { return end(); }
	reverse_iterator rend() noexcept { return begin(); }

	const_reverse_iterator rbegin() const noexcept { return end(); }
	const_reverse_iterator rend() const noexcept { return begin(); }

	const_reverse_iterator crbegin() const noexcept { return end(); }
	const_reverse_iterator crend() const noexcept { return begin(); }

	// capacity

	index_type width() const noexcept { return m_width; }
	index_type height() const noexcept { return m_height; }

	size_type size() const noexcept { return static_cast<size_type>( m_width * m_height ); }
	difference_type ssize() const noexcept { return static_cast<difference_type>( m_width * m_height ); }

	// modifiers

	template <typename U>
	void set( index_type x, index_type y, U&& value )
	{
		dbExpects( 0 <= x && x < stdx::narrow_cast<index_type>( m_width ) );
		dbExpects( 0 <= y && y < stdx::narrow_cast<index_type>( m_height ) );
		m_data[ get_pos( x, y ) ] = std::forward<U>( value );
	}

	void clear() noexcept
	{
		m_data.reset();
		m_width = 0;
		m_height = 0;
	}

	void swap( array2& other ) noexcept
	{
		auto temp = std::move( *this );
		*this = std::move( other );
		other = std::move( temp );
	}

	void resize( index_type w, index_type h )
	{
		dbExpects( w >= 0 && h >= 0 );

		if ( m_width == w && m_height == h )
			return;

		if ( w <= m_width && h <= m_height )
		{
			// no zero init
			array2 newArray;
			newArray.overwriteAllocate( w, h );
			newArray.copy( 0, 0, *this, w, h, 0, 0 );
			swap( newArray );
		}
		else
		{
			// must zero init
			array2 newArray( w, h );
			newArray.copy( 0, 0, *this, ( std::min )( m_width, w ), ( std::min )( m_height, h ), 0, 0 );
			swap( newArray );
		}
	}

	void fill( const T& value )
	{
		for ( T& element : *this )
			element = value;
	}

	void fill( index_type left, index_type top, index_type w, index_type h, const T& value )
	{
		dbExpects( left >= 0 );
		dbExpects( top >= 0 );
		dbExpects( left + w <= m_width );
		dbExpects( top + h <= m_height );
		dbExpects( w >= 0 );
		dbExpects( h >= 0 );

		const auto rowStart = begin() + ( top * m_width ) + left;
		const auto rowEnd = rowStart + ( h * m_width );

		for ( auto row = rowStart; row != rowEnd; row += m_width )
		{
			std::fill_n( row, w, value );
		}
	}

	template <typename U>
	void copy(
		index_type destX,
		index_type destY,
		const array2<U>& other,
		index_type left,
		index_type top,
		index_type w,
		index_type h )
	{
		dbExpects( 0 <= left && left + w <= other.m_width );
		dbExpects( 0 <= top && top + h <= other.m_height );
		dbExpects( 0 <= destX && destX + w <= m_width );
		dbExpects( 0 <= destY && destY + h <= m_height );
		dbExpects( 0 <= w );
		dbExpects( 0 <= h );

		auto destRow = begin() + ( destY * m_width ) + destX;
		const auto destRowEnd = destRow + ( h * m_width );

		auto srcRow = other.begin() + ( top * other.m_width ) + left;

		for ( ; destRow != destRowEnd; destRow += m_width, srcRow += other.m_width )
		{
			std::copy( srcRow, srcRow + w, destRow );
		}
	}

	// lookup

	index_type get_x( size_type pos ) const noexcept
	{
		dbExpects( pos < size() );
		return static_cast<index_type>( pos % m_width );
	}

	index_type get_y( size_type pos ) const noexcept
	{
		dbExpects( pos < size() );
		return static_cast<index_type>( pos / m_width );
	}

	index_type get_x( const_iterator it ) const noexcept
	{
		return get_x( static_cast<size_type>( it - cbegin() ) );
	}

	index_type get_y( const_iterator it ) const noexcept
	{
		return get_y( static_cast<size_type>( it - cbegin() ) );
	}

	size_type get_pos( index_type x, index_type y ) const noexcept
	{
		dbExpects( x >= 0 );
		dbExpects( x < stdx::narrow_cast<index_type>( m_width ) );
		dbExpects( y >= 0 );
		dbExpects( y < stdx::narrow_cast<index_type>( m_height ) );
		return static_cast<size_type>( x + y * m_width );
	}

	// comparison

	friend bool operator==( const array2& lhs, const array2& rhs )
	{
		if ( lhs.m_width != rhs.m_width || lhs.m_height != rhs.m_height )
			return false;

		auto[ lhsIt, rhsIt ] = std::mismatch( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );
		dbAssert( ( lhsIt == lhs.end() ) == ( rhsIt == rhs.end() ) );
		return ( lhsIt == lhs.end() );
	}

	friend bool operator!=( const array2& lhs, const array2& rhs )
	{
		return !( lhs == rhs );
	}

private:
	void defaultAllocate( size_type w, size_type h )
	{
		const auto dataSize = w * h;
		if ( dataSize == 0 )
			m_data.reset();
		else
			m_data = std::make_unique<T[]>( dataSize );

		m_width = w;
		m_height = h;
	}

	void overwriteAllocate( size_type w, size_type h )
	{
		const auto dataSize = w * h;
		if ( dataSize == 0 )
			m_data.reset();
		else
			m_data = stdx::make_unique_for_overwrite<T[]>( dataSize );

		m_width = w;
		m_height = h;
	}

private:
	std::unique_ptr<T[]> m_data;
	index_type m_width = 0;
	index_type m_height = 0;
};

} // namespace stdx

