#pragma once

#include <stdx/memory.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace stdx
{

template <typename T>
class dynamic_array
{
public:
	using value_type = T;
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

	// construction/assignment

	dynamic_array() noexcept = default;

	dynamic_array( const dynamic_array& other )
		: dynamic_array{ other.m_size }
		, m_size{ other.m_size }
	{
		std::copy( other.begin(), other.end(), begin() );
	}

	dynamic_array( dynamic_array&& other ) noexcept
		: m_data{ std::move( other.m_data ) }
		, m_size( std::exchange( other.m_size, 0 ) )
	{}

	dynamic_array( size_type size )
		: m_data{ stdx::make_unique_for_overwrite<T[]>( size ) }
		, m_size{ size }
	{}

	dynamic_array( size_type size, const T& value )
		: m_data{ stdx::make_unique_for_overwrite<T[]>( size ) }
		, m_size{ size }
	{
		fill( value );
	}

	dynamic_array( T* data, size_type size ) noexcept
		: m_data{ data }
		, m_size{ size }
	{}

	dynamic_array( std::unique_ptr<T> data, size_type size ) noexcept
		: m_data{ std::move( data ) }
		, m_size{ size }
	{}

	~dynamic_array() = default;

	dynamic_array& operator=( const dynamic_array& other )
	{
		*this = dynamic_array{ other };
	}

	dynamic_array& operator=( dynamic_array&& other )
	{
		m_data = std::move( other.m_data );
		m_size = std::exchange( other.m_size, 0 );
	}

	// access

	reference at( size_type pos )
	{
		if ( pos >= m_size )
			throw std::out_of_range{};

		return m_data[ pos ];
	}

	const_reference at( size_type pos ) const
	{
		if ( pos >= m_size )
			throw std::out_of_range{};

		return m_data[ pos ];
	}

	reference operator[]( size_type pos ) noexcept
	{
		dbExpects( pos < m_size );
		return m_data[ pos ];
	}

	const_reference operator[]( size_type pos ) const noexcept
	{
		dbExpects( pos < m_size );
		return m_data[ pos ];
	}

	reference front() noexcept
	{
		dbExpects( m_size != 0 );
		return m_data[ 0 ];
	}

	const_reference front() const noexcept
	{
		dbExpects( m_size != 0 );
		return m_data[ 0 ];
	}

	reference back() noexcept
	{
		dbExpects( m_size != 0 );
		return m_data[ m_size - 1 ];
	}

	const_reference back() const noexcept
	{
		dbExpects( m_size != 0 );
		return m_data[ m_size - 1 ];
	}

	pointer data() noexcept
	{
		return m_data.get();
	}

	const_pointer data() const noexcept
	{
		return m_data.get();
	}

	// iterators

	iterator begin() noexcept { return m_data.get(); }
	iterator end() noexcept { return m_data.get() + m_size; }

	const_iterator begin() const noexcept { return m_data.get(); }
	const_iterator end() const noexcept { return m_data.get() + m_size; }

	const_iterator cbegin() const noexcept { return m_data.get(); }
	const_iterator cend() const noexcept { return m_data.get() + m_size; }

	reverse_iterator rbegin() noexcept { return end(); }
	reverse_iterator rend() noexcept { return begin(); }

	const_reverse_iterator rbegin() const noexcept { return end(); }
	const_reverse_iterator rend() const noexcept { return begin(); }

	const_reverse_iterator crbegin() const noexcept { return end(); }
	const_reverse_iterator crend() const noexcept { return begin(); }

	// capacity

	size_type size() const noexcept
	{
		return m_size;
	}

	size_type max_size() const noexcept
	{
		return m_size;
	}

	// operations

	void fill( const T& value )
	{
		for ( auto& element : *this )
		{
			element = value;
		}
	}

	void swap( dynamic_array& other )
	{
		auto temp = std::move( *this );
		*this = std::move( other );
		other = std::move( temp );
	}

	friend bool operator==( const dynamic_array& lhs, const dynamic_array& rhs )
	{
		if ( lhs.size() != rhs.size() )
			return false;

		auto[ lhsIt, rhsIt ] = std::mismatch( lhs.begin(), lhs.end(), rhs.begin() );
		return lhsIt == lhs.end();
	}

	friend bool operator!=( const dynamic_array& lhs, const dynamic_array& rhs )
	{
		return !( lhs == rhs );
	}

	friend bool operator<( const dynamic_array& lhs, const dynamic_array& rhs )
	{
		return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );
	}

	friend bool operator>( const dynamic_array& lhs, const dynamic_array& rhs )
	{
		return rhs < lhs;
	}

	friend bool operator<=( const dynamic_array& lhs, const dynamic_array& rhs )
	{
		return !( lhs > rhs );
	}

	friend bool operator>=( const dynamic_array& lhs, const dynamic_array& rhs )
	{
		return !( lhs < rhs );
	}

private:
	std::unique_ptr<T> m_data;
	size_type m_size = 0;
};

} // namespace stdx