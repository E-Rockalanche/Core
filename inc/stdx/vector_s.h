#ifndef STDX_VECTOR_S_HPP
#define STDX_VECTOR_S_HPP

#include <iterator>
#include <stdexcept>
#include <type_traits>

namespace stdx
{

template <typename T, size_t SIZE>
class alignas( T ) alignas( T* ) vector_s
{
public:
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;

	using size_type = size_t;
	using difference_type = ptrdiff_t;

	using iterator = T*;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
	vector_s() noexcept : m_end( m_data ) {}
	vector_s( size_type count ) { resize( count ); }
	vector_s( size_type count, const T& value ) { resize( count, value ); }

	template <typename InputIt>
	vector_s( typename std::enable_if< !std::is_integral_v< InputIt >, InputIt >::type first, InputIt last )
		: vector_s()
	{
		insert( begin(), first, last );
	}

	vector_s( const vector_s& other ) : vector_s() { insert( begin(), other.cbegin(), other.cend() ); }
	vector_s( std::initializer_list<T> init ) : vector_s() { insert(begin(), init); }

	~vector_s() { clear(); } // destroy objects in raw char buffer

	vector_s& operator=( const vector_s& other )
	{
		clear();
		insert( begin(), other.cbegin(), other.cend() );
		return *this;
	}

	vector_s& operator=( std::initializer_list<T> iList )
	{
		clear();
		insert( begin(), iList.begin(), iList.end() );
		return *this;
	}

	void assign( size_type count, const T& value )
	{
		clear();
		resize( count, value );
	}

	template <class InputIt>
	std::enable_if_t< !std::is_integral_v< InputIt >, void >
	assign( InputIt first, InputIt last )
	{
		clear();
		insert( begin(), first, last );
		return *this;
	}

	void assign( std::initializer_list<T> iList )
	{
		assign( iList.begin(), iList.end() );
	}

	T& at( size_type index )
	{
		if ( index >= size() )
			throw std::out_of_range( out_of_range_message );
		return data()[ index ];
	}

	const T& at( size_type index ) const
	{
		if ( index >= size() )
			throw std::out_of_range( out_of_range_message );
		return data()[ index ];
	}

	T& operator[]( size_type index ) noexcept { return m_data[ index ]; }
	const T& operator[]( size_type index ) const noexcept { return m_data[ index ]; }

	T& front() noexcept { return m_data[ 0 ]; }
	const T& front() const noexcept { return m_data[ 0 ]; }

	T& back() noexcept { return *( m_end - 1 ); }
	const T& back() const noexcept { return *( m_end - 1 ); }

	T* data() noexcept { return m_data; }
	const T* data() const noexcept { return m_data; }

	iterator begin() noexcept { return m_data; }
	iterator end() noexcept { return m_end; }

	const_iterator cbegin() const noexcept { return m_data; }
	const_iterator cend() const  noexcept { return m_end; }

	reverse_iterator rbegin() noexcept { return reverse_iterator( end() ); }
	reverse_iterator rend() noexcept { return reverse_iterator( begin() ); }

	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator( cend() ); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator( cbegin() ); }

	bool empty() const noexcept { return m_data == m_end; }
	size_type size() const noexcept { return static_cast<size_type>( m_end - m_data ); }
	size_type max_size() const noexcept { return SIZE; }
	size_type capacity() const noexcept { return SIZE; }

	void clear()
	{
		destroy( begin(), end() );
		m_end = m_data;
	}

	iterator insert( const_iterator pos_, const T& value )
	{
		iterator pos = const_cast<iterator>( pos_ );
		if ( pos == end() )
		{
			push_back( value );
		}
		else
		{
			shift_back( pos, 1 );
			*pos = value;
		}
		return pos + 1;
	}

	iterator insert( const_iterator pos_, T&& value )
	{
		iterator pos = const_cast<iterator>( pos_ );
		if ( pos == end() )
		{
			push_back( std::move( value ) );
		}
		else
		{
			shift_back( pos, 1 );
			*pos = value;
		}
		return pos + 1;
	}

	iterator insert( const_iterator pos_, size_type count, const T& value )
	{
		iterator pos = const_cast<iterator>(pos_);

		if ( count == 0 )
			return pos;

		if ( pos == end() )
		{
			resize( size() + count, value );
		}
		else
		{
			auto prev_end = end();
			shift_back( pos, count );
			auto it = pos;
			// copy into existing positions
			for( ; it != prev_end; ++it )
				*it = value;
			// copy construct into new positions
			for( ; it != pos + count; ++it )
				new ( it ) T( value );
		}
		return pos + 1;
	}

	template <typename InputIt>
	typename std::enable_if< !std::is_integral_v< InputIt >, iterator >::type
	insert( const_iterator pos_, InputIt first, InputIt last )
	{
		iterator pos = const_cast<iterator>( pos_ );

		if ( first == last )
			return pos;

		if ( pos == end() )
		{
			auto src = first;
			auto dest = pos;
			for( ; src != last; ++src, ++dest )
				new ( dest ) T( *src );
		}
		else
		{
			auto prev_end = end();
			size_type count = last - first;
			shift_back( pos, count );
			auto dest = pos;
			auto src = first;
			// copy into existing positions
			for( ; dest != prev_end; ++dest, ++src )
				*dest = *src;
			// copy construct into new positions
			for( ; dest != pos + count; ++dest, ++src )
				new ( dest ) T( *src );
		}

		return pos + 1;
	}

	iterator insert( const_iterator pos_, std::initializer_list<T> iList )
	{
		iterator pos = const_cast<iterator>( pos_ );

		if ( iList.begin() == iList.end() )
			return pos;

		if ( pos == end() )
		{
			auto src = iList.begin();
			auto dest = pos;
			for( ; src != iList.end(); ++src, ++dest )
				new ( dest ) T( *src );
		}
		else
		{
			auto prev_end = end();
			size_type count = iList.end() - iList.begin();
			shift_back( pos, count );
			auto dest = pos;
			auto src = iList.begin();
			// copy into existing positions
			for( ; dest != prev_end; ++dest, ++src )
				*dest = *src;
			// copy construct into new positions
			for( ; dest != pos + count; ++dest, ++src )
				new ( dest ) T( *src );
		}

		return pos + 1;
	}

	template <class... Args>
	iterator emplace( const_iterator pos_, Args&&... args )
	{
		iterator pos = const_cast<iterator>( pos_ );
		if ( pos == end() )
		{
			emplace_back( std::forward<Args>( args )... );
		}
		else
		{
			shift_back( pos, 1 );
			( *pos ).~T();
			new ( pos ) T( std::forward<Args>( args )... );
		}
		return pos;
	}

	iterator erase( const_iterator pos_ )
	{
		iterator pos = const_cast<iterator>(pos_);
		if ( pos >= end() )
			return end();
		else if ( pos == end() - 1 )
			pop_back();
		else
			shift_forward( pos, 1 );
		
		return pos;
	}

	iterator erase( const_iterator first_, const_iterator last_ )
	{
		iterator first = const_cast<iterator>(first_);
		iterator last = const_cast<iterator>(last_);
		if ( last == end() )
			resize( first - begin() );
		else
			shift_forward( last, last - first );
		
		return first;
	}

	void push_back( const T& value )
	{
		checkSize( 1 );
		new ( m_end++ ) T( value );
	}

	void push_back( T&& value )
	{
		checkSize( 1 );
		new ( m_end++ ) T( std::move( value ) );
	}

	template <class... Args>
	T& emplace_back( Args&&... args )
	{
		checkSize( 1 );
		new ( m_end ) T( std::forward<Args>( args )... );
		return *( m_end++ );
	}

	void pop_back() { ( --m_end ).~T(); }

	void resize( size_type count ) // construct additional elements in place
	{
		if ( count > capacity() )
			throw std::length_error( exceed_capacity_message );
		
		else if ( count > size() )
		{
			for(auto it = end(); it != begin() + count; ++it)
				new ( it ) T();
		}
		else if ( count < size() )
		{
			destroy( begin() + count, end() );
		}
		m_end = m_data + count;
	}

	void resize( size_type count, const T& value )
	{
		if ( count > capacity() )
			throw std::length_error( exceed_capacity_message );
		
		if ( count > size() )
		{
			for(auto it = end(); it != begin() + count; ++it)
				new ( it ) T( value );
		}
		else if ( count < size() )
		{
			destroy( begin() + count, end() );
		}
		m_end = m_data + count;
	}

private:
	void destroy( const_iterator first, const_iterator last )
	{
		for( auto it = first; it != last; ++it )
			( *it ).~T();
	}

	void shift_back( const_iterator first_, size_type distance )
	{
		iterator first = const_cast<iterator>(first_);
		if ( first + distance >= begin() + capacity() )
			throw std::length_error( exceed_capacity_message );

		auto last = end() - 1;
		auto new_first = first + distance;
		auto dest = last + distance;
		auto src = last;
		// move construct into new positions
		for( ; dest != last; --dest, --src)
			new ( dest ) T( std::move( *src ) );
		// move into existing posiitons
		for( ; dest != new_first; --dest, --src)
			*dest = std::move( *src );
		m_end += distance;
	}

	void shift_forward( const_iterator first_, size_type distance )
	{
		iterator first = const_cast<iterator>(first_);
		auto src = first;
		auto dest = first - distance;
		for( ; src != end(); ++src, ++dest )
			*dest = std::move( *src );
		destroy( end() - distance, end() );
		m_end -= distance;
	}

	void checkSize( size_type count ) const
	{
		if ( size() + count > SIZE )
			throw std::length_error( exceed_capacity_message );
	}

private:
	T* m_end = nullptr;

	union
	{
		T m_data[ SIZE ];
		char m_bytes[ sizeof( T ) * SIZE ];
	};

	static constexpr const char* out_of_range_message = "vector_s index out of range";
	static constexpr const char* exceed_capacity_message = "vector_s size exceeds capacity";
};

}

#endif