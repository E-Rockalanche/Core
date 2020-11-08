#pragma once

#include <stdx/assert.h>

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
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;

	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	// construction/assignment

	dynamic_array() noexcept = default;

	dynamic_array( const dynamic_array& other ) : dynamic_array( other.m_size )
	{
		std::copy( other.begin(), other.end(), m_data );
	}

	dynamic_array( dynamic_array&& other ) noexcept
		: m_data{ std::exchange( other.m_data, nullptr ) }
		, m_size( std::exchange( other.m_size, 0 ) )
	{}

	// memory left uninitialized
	explicit dynamic_array( size_type size_ )
		: m_data{ new T[ size_ ] }
		, m_size{ size_ }
	{}

	dynamic_array( size_type size_, const T& value )
		: m_data{ new T[ size_ ] }
		, m_size{ size_ }
	{
		fill( value );
	}

	// takes ownership of data
	dynamic_array( T* data_, size_type size_ ) noexcept
		: m_data{ data_ }
		, m_size{ size_ }
	{
		dbExpects( ( data_ == nullptr ) == ( size_ == 0 ) );
	}

	dynamic_array( std::initializer_list<T> init )
		: m_data{ new T[ init.size() ] }, m_size{ init.size() }
	{
		std::copy( init.begin(), init.end(), m_data );
	}

	template <typename InputIt>
	dynamic_array( InputIt first, InputIt last )
	{
		m_size = static_cast<size_type>( std::distance( first, last ) );
		m_data = new T[ m_size ];
		std::copy( first, last, m_data );
	}

	~dynamic_array()
	{
		clear();
	}

	dynamic_array& operator=( const dynamic_array& other )
	{
		return *this = dynamic_array{ other };
	}

	dynamic_array& operator=( dynamic_array&& other ) noexcept
	{
		clear();
		m_data = std::exchange( other.m_data, nullptr );
		m_size = std::exchange( other.m_size, 0 );
		return *this;
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
		return m_data;
	}

	const_pointer data() const noexcept
	{
		return m_data;
	}

	// iterators

	iterator begin() noexcept { return m_data; }
	iterator end() noexcept { return m_data + m_size; }

	const_iterator begin() const noexcept { return m_data; }
	const_iterator end() const noexcept { return m_data + m_size; }

	const_iterator cbegin() const noexcept { return m_data; }
	const_iterator cend() const noexcept { return m_data + m_size; }

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

	bool empty() const noexcept
	{
		return m_size == 0;
	}

	// modifiers

	void clear()
	{
		if ( m_data )
		{
			delete[] m_data;
			m_data = nullptr;
			m_size = 0;
		}
	}

	// moves data to newly allocated array of size newSize
	void resize( size_type newSize )
	{
		if ( newSize != m_size )
		{
			if ( newSize > 0 )
			{
				T* newData = resize_imp( newSize );

				delete[] m_data;
				m_data = newData;
				m_size = newSize;
			}
			else
			{
				clear();
			}
		}
	}

	// moves data to newly allocated array of size newSize
	// if newSize is larger than size, fill range [size, newSize-1] with value
	void resize( size_type newSize, const T& value )
	{
		if ( newSize != m_size )
		{
			if ( newSize > 0 )
			{
				T* newData = resize_imp( newSize );

				if ( newSize > m_size )
					std::fill( newData + m_size, newData + newSize, value );

				delete[] m_data;
				m_data = newData;
				m_size = newSize;
			}
			else
			{
				clear();
			}
		}
	}

	void fill( const T& value )
	{
		auto it = m_data;
		const auto last = m_data + m_size;
		for ( ; it != last; ++it )
			*it = value;
	}

	void swap( dynamic_array& other ) noexcept
	{
		std::swap( m_data, other.m_data );
		std::swap( m_size, other.m_size );
	}

	void reset( T* data_, size_type size_ )
	{
		dbExpects( ( data_ == nullptr ) == ( size_ == 0 ) );
		clear();
		m_data = data_;
		m_size = size_;
	}

	T* release() noexcept
	{
		m_size = 0;
		return std::exchange( m_data, nullptr );
	}

private:
	T* resize_imp( size_type newSize )
	{
		T* newData = new T[ newSize ];
		std::move( begin(), begin() + ( std::min )( m_size, newSize ), newData );
		return newData;
	}

private:
	T* m_data = nullptr;
	size_type m_size = 0;
};

// comparison

template <typename T>
bool operator==( const dynamic_array<T>& lhs, const dynamic_array<T>& rhs )
{
	if ( lhs.size() != rhs.size() )
		return false;

	auto[ lhs_it, rhs_it ] = std::mismatch( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );
	return lhs_it == lhs.end();
}

template <typename T>
inline bool operator!=( const dynamic_array<T>& lhs, const dynamic_array<T>& rhs )
{
	return !( lhs == rhs );
}

template <typename T>
bool operator<( const dynamic_array<T>& lhs, const dynamic_array<T>& rhs )
{
	return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );
}

template <typename T>
inline bool operator>( const dynamic_array<T>& lhs, const dynamic_array<T>& rhs )
{
	return rhs < lhs;
}

template <typename T>
inline bool operator<=( const dynamic_array<T>& lhs, const dynamic_array<T>& rhs )
{
	return !( lhs > rhs );
}

template <typename T>
inline bool operator>=( const dynamic_array<T>& lhs, const dynamic_array<T>& rhs )
{
	return !( lhs < rhs );
}

} // namespace stdx