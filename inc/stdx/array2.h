#pragma once

#include <stdx/memory.h>

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
	using pointer = T * ;
	using const_pointer = const T*;
	using reference = T & ;
	using const_reference = const T&;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using index_type = size_type;

	array2() noexcept = default;

	array2( size_type w, size_type h )
	{
		defaultAllocate( w, h );
	}

	array2( size_type w, size_type h, const T& value )
	{
		overwriteAllocate( w, h );
		fill( value );
	}

	array2( array2&& ) noexcept = default;

	array2( const array2& other )
	{
		overwriteAllocate( other.width(), other.height() );
		std::copy( other.begin(), other.end(), begin() );
	}

	array2& operator=( array2&& other ) = default;

	array2& operator=( const array2& other )
	{
		if ( m_width != other.width() && m_height != other.height() )
			overwriteAllocate( other.width(), other.height() );

		std::copy( other.begin(), other.end(), begin() );
	}

	size_type width() const noexcept { return m_width; }
	size_type height() const noexcept { return m_height; }

	T* data() noexcept { return m_data.get(); }
	const T* data() const noexcept { return m_data.get(); }

	size_type size() const noexcept { return m_width * m_height; }
	difference_type ssize() const noexcept { return static_cast<difference_type>( m_width * m_height ); }

	T& get( index_type x, index_type y ) noexcept
	{
		dbAssert( 0 <= x && x < m_width );
		dbAssert( 0 <= y && y < m_height );
		return m_data[ x + y * m_width ];
	}

	const T& get( index_type x, index_type y ) const noexcept
	{
		dbAssert( 0 <= x && x < m_width );
		dbAssert( 0 <= y && y < m_height );
		return m_data[ x + y * m_width ];
	}

	template <typename U>
	void set( index_type x, index_type y, U&& value )
	{
		dbAssert( 0 <= x && x < m_width );
		dbAssert( 0 <= y && y < m_height );
		m_data[ x + y * m_width ] = std::forward<U>( value );
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

	void resize( size_type w, size_type h )
	{
		if ( m_width == w && m_height == h )
			return;

		if ( w <= m_width && h <= m_height )
		{
			// no zero init
			array2 newArray;
			newArray.overwriteAllocate( w, h );
			newArray.copy( *this, 0, 0, w, h, 0, 0 );
			swap( newArray );
		}
		else
		{
			// must zero init
			array2 newArray( w, h );
			newArray.copy( *this, 0, 0, ( std::min )( m_width, w ), ( std::min )( m_height, h ), 0, 0 );
			swap( newArray );
		}
	}

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

	void fill( const T& value )
	{
		for ( T& element : *this )
			element = value;
	}

	void fill( index_type left, index_type top, size_type w, size_type h, const T& value )
	{
		dbAssert( left >= 0 );
		dbAssert( top >= 0 );
		dbAssert( left + w <= width() );
		dbAssert( top + h <= height() );

		const auto rowStart = begin() + ( top * m_width ) + left;
		const auto rowEnd = rowStart + ( h * m_width );

		for ( auto row = rowStart; row != rowEnd; row += m_width )
		{
			std::fill_n( row, w, value );
		}
	}

	template <typename U>
	void copy(
		const array2<U>& other,
		index_type left,
		index_type top,
		size_type w,
		size_type h,
		index_type destX,
		index_type destY )
	{
		dbAssert( 0 <= left && left + w <= other.width() );
		dbAssert( 0 <= top && top + h <= other.height() );
		dbAssert( 0 <= destX && destX + w <= width() );
		dbAssert( 0 <= destY && destY + h <= height() );

		auto destRow = begin() + ( destY * m_width ) + destX;
		const auto destRowEnd = destRow + ( h * m_width );

		auto srcRow = other.begin() + ( top * other.m_width ) + left;

		for ( ; destRow != destRowEnd; destRow += m_width, srcRow += other.m_width )
		{
			std::copy( srcRow, srcRow + w, destRow );
		}
	}

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
		m_data = std::make_unique<T[]>( w * h );
		m_width = w;
		m_height = h;
	}

	void overwriteAllocate( size_type w, size_type h )
	{
		m_data = stdx::make_unique_for_overwrite<T[]>( w * h );
		m_width = w;
		m_height = h;
	}

private:
	std::unique_ptr<T[]> m_data;
	size_type m_width = 0;
	size_type m_height = 0;
};

} // namespace stdx

