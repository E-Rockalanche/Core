#pragma once

#include <algorithm>
#include <iterator>

template <typename T>
class array2
{
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
	using size_type = std::size_t;
	using index_type = std::ptrdiff_t;

	array2() noexcept = default;

	array2( size_type w, size_type h )
	{
		defaultAllocate( w, h );
	}

	array2( array2&& ) noexcept = default;

	template <typename U>
	array2( const array2<U>& other )
	{
		overwriteAllocate( other.width(), other.height() );
		std::copy( other.begin(), other.end(), data() );
	}

	array2& operator=( array2&& ) = default;

	template <typename U>
	array2& operator=( const array2<U>& other )
	{
		if ( m_width != other.width() && m_height != other.height() )
			overwriteAllocate( other.width(), other.height() );

		std::copy( other.begin(), other.end(), data() );
	}
	
	size_type width() const noexcept { return m_width; }
	size_type height() const noexcept { return m_height; }

	T* data() noexcept { return m_data.get(); }
	const T* data() const noexcept { return m_data.get(); }

	size_type size() const noexcept { return m_width * m_height; }

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
	void set( index_type x, index_type y, U&& value ) noexcept
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

		array2 newArray( w, h );
		newArray.copy( *this, 0, 0, w, h, 0, 0 );
		swap( newArray );
	}

	T* begin() noexcept { return m_data.get(); }
	T* end() noexcept { return begin() + size(); }

	const T* begin() const noexcept { return m_data.get(); }
	const T* end() const noexcept { return begin() + size(); }

	const T* cbegin() const noexcept { return begin(); }
	const T* cend() const noexcept { return end(); }

	T* begin() noexcept { return m_data.get(); }
	T* end() noexcept { return begin() + size(); }

	const T* begin() const noexcept { return m_data.get(); }
	const T* end() const noexcept { return begin() + size(); }

	const T* cbegin() const noexcept { return begin(); }
	const T* cend() const noexcept { return end(); }

	void fill( const T& value )
	{
		for ( T& element : *this )
			element = value;
	}

	template <typename U>
	void copy(
		const array2<U>& other,
		index_type left,	index_type top,
		size_type w,		size_type h,
		index_type destX,	index_type destY );

private:
	void defaultAllocate( size_type w, size_type h )
	{
		m_data = std::make_unique<T[]>( w * h );
		m_width = w;
		m_height = h;
	}

	void overwriteAllocate( size_type w, size_type h )
	{
		m_data = std::make_unique_for_overwrite<T[]>( w * h );
		m_width = w;
		m_height = h;
	}

private:
	std::unique_ptr<T[]> m_data;
	size_type m_width = 0;
	size_type m_height = 0;
};

template <typename T>
template <typename U>
void array2<T>::copy(
	const array2<U>& other,
	index_type left,	index_type top,
	size_type w,		size_type h,
	index_type destX,	index_type destY )
{
	dbAssert( 0 <= left && left + w <= other.width() );
	dbAssert( 0 <= top && top + h <= other.height() );
	dbAssert( 0 <= destX && destX + w <= width() );
	dbAssert( 0 <= destY && destY + h <= height() );

	for ( size_type y = 0; y < h; ++y )
	{
		T* destRow = data() + y * width();
		const U* srcRow = other.data() + y * other.width();
		std::copy( srcRow + left, srcRow + left + w, destRow + destX );
	}
}