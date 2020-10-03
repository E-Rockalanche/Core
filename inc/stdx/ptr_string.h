#pragma once

#include <stdx/assert.h>
#include <stdx/int.h>

#include <algorithm>
#include <string>

namespace stdx
{

// single pointer size, null terminated string object
// data is always heap allocated
template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_ptr_string
{
public:
	using traits_type = Traits;
	using value_type = CharT;

	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using reference = CharT&;
	using const_reference = const CharT&;
	using pointer = CharT&;
	using const_pointer = const CharT*;

	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	using view_type = std::basic_string_view<CharT, Traits>;

	static constexpr size_type npos = std::numeric_limits<size_type>::max();

	// construction/assignment

	basic_ptr_string() noexcept = default;

	basic_ptr_string( size_type count, CharT c )
		: m_storage{ storage::create( count, count ) }
	{
		Traits::assign( m_storage->data(), count, c );
		dbAssert( *m_storage->end() == '\0' );
	}

	basic_ptr_string( const basic_ptr_string& other, size_type pos, size_type count = npos )
	{
		if ( other.m_storage )
		{
			const auto n = std::min( other.m_storage->size - pos, count );
			m_storage = storage::create( n, n );
			Traits::copy( m_storage->data(), other.m_storage.data() + pos, n );
			dbAssert( *m_storage->end() == '\0' );
		}
	}

	basic_ptr_string( const CharT* s, size_type count )
		: m_storage{ storage::create( count, count ) }
	{
		dbAssert( s != nullptr );
		Traits::copy( m_storage->data(), s, count );
		dbAssert( *m_storage->end() == '\0' );
	}

	basic_ptr_string( const CharT* s )
	{
		dbAssert( s != nullptr );
		const auto n = Traits::length( s );
		m_storage = storage::create( n, n );
		Traits::copy( m_storage->data(), s, n );
		dbAssert( *m_storage->end() == '\0' );
	}

	template <typename InputIt>
	basic_ptr_string( InputIt first, const InputIt last )
	{
		const auto n = stdx::narrow_cast<size_type>( std::distance( first, last ) );
		m_storage = storage::create( n, n );
		auto dest = m_storage.data();
		for ( ; first != last; ++dest, ++first )
			Traits::assign( *dest, *first );

		dbAssert( *m_storage->end() == '\0' );
	}

	basic_ptr_string( const basic_ptr_string& other )
	{
		if ( other.m_storage != nullptr )
		{
			m_storage = storage::create( other.m_storage->size, other.m_storage->size );
			Traits::copy( m_storage->data(), other.m_storage.data(), other.m_storage->size );
			dbAssert( *m_storage->end() == '\0' );
		}
	}

	basic_ptr_string( basic_ptr_string&& other ) noexcept
		: m_storage{ std::exchange( other.m_storage, nullptr ) }
	{}

	basic_ptr_string( std::initializer_list<CharT> init )
		: basic_ptr_string( init.begin(), init.end() )
	{}

	template <typename T>
	explicit basic_ptr_string( const T& t )
	{
		const auto n = std::size( t );
		m_storage = storage::create( n, n );
		auto dest = m_storage->data();
		const auto last = dest + n;
		for ( auto src = std::begin( t ); dest != last; ++dest, ++src )
			Traits::assign( *dest, *src );

		dbAssert( *m_storage->end() == '\0' );
	}

	template <typename T>
	explicit basic_ptr_string( const T& t, size_type pos, size_type count = npos )
	{
		dbAssert( pos <= std::size( t ) );
		const auto n = std::min( std::size( t ) - pos, count );
		m_storage = storage::create( n, n );
		auto dest = m_storage->data();
		const auto last = dest + n;
		for ( auto src = std::begin( t ) + pos; dest != last; ++dest, ++src )
			Traits::assign( *dest, *src );

		dbAssert( *m_storage->end() == '\0' );
	}

	basic_ptr_string& operator=( const basic_ptr_string& other )
	{
		if ( m_storage && other.m_storage &&  m_storage->capacity >= other.m_storage->size )
		{
			Traits::copy( m_storage->data(), other.m_storage->data(), other.m_storage->size + 1 );
			m_storage->size = other.m_storage->size;
		}
		else
		{
			*this = basic_ptr_string( other );
		}
		return *this;
	}

	basic_ptr_string& operator=( basic_ptr_string&& other ) noexcept
	{
		if ( m_storage )
			delete m_storage;

		m_storage = std::exchange( other.m_storage, nullptr );
	}

	basic_ptr_string& operator=( const CharT* s )
	{
		const auto n = Traits::length( s );
		if ( m_storage && m_storage->capacity >= n )
		{
			m_storage->size = n;
			Traits::copy( m_storage->data(), s, n + 1 );
		}
		else
		{
			*this = basic_ptr_string( s );
		}

		return *this;
	}

	basic_ptr_string& operator=( CharT c )
	{
		if ( m_storage )
		{
			m_storage->size = 1;
			auto dest = m_storage->data();
			Traits::assign( *dest, c );
			*++dest = '\0';
		}
		else
		{
			*this = basic_ptr_string( &c, 1 );
		}
		return *this;
	}

	basic_ptr_string& operator=( std::initializer_list<CharT> init )
	{
		const auto n = std::size( init );
		if ( m_storage && m_storage->capacity >= n )
		{
			m_storage->size = n;
			Traits::copy( m_storage->data(), std::begin( init ), n );
			*m_storage->end() = '\0';
		}
		else
		{
			*this = basic_ptr_string( init );
		}
		return *this;
	}

	template <typename T>
	basic_ptr_string& operator=( const T& t )
	{
		const auto n = std::size( t );
		if ( m_storage && m_storage->capacity > n )
		{
			m_storage->size = n;
			auto dest = m_storage->data();
			const auto last = dest + n;
			for ( auto src = std::begin( t ); dest != last; ++dest, ++src )
				Traits::assign( *dest, *src );

			*dest = '\0';
		}
		else
		{
			*this = basic_ptr_string( t );
		}
		return *this;
	}

	basic_ptr_string& assign( size_type count, CharT c )
	{
		if ( m_storage && m_storage->capacity >= count )
		{
			m_storage->size = count;
			auto dest = m_storage->data();
			const auto last = dest + count;
			for ( ; dest != last; ++dest )
				Traits::assign( *dest, c );

			*dest = '\0';
		}
		else
		{
			*this = basic_ptr_string( count, c );
		}
		return *this;
	}

	basic_ptr_string& assign( const basic_ptr_string& other )
	{
		return *this = other;
	}

	basic_ptr_string& assign( const basic_ptr_string& other, size_type pos, size_type count = npos )
	{
		dbAssert( pos <= other.size() );
		if ( m_storage && other.m_storage && m_storage->capacity >= other.m_storage->size )
		{
			m_storage->size = other.m_storage->size();
			Traits::copy( m_storage->data(), other.m_storage->data(), other.m_storage->size() + 1 );
		}
		else
		{
			*this = basic_ptr_string( other, pos, count );
		}
		return *this;
	}

	basic_ptr_string& assign( basic_ptr_string&& other )
	{
		return *this = std::move( other );
	}

	basic_ptr_string& assign( const CharT* s, size_type count )
	{
		if ( m_storage && m_storage->capacity >= count )
		{
			m_storage->size = count;
			Traits::copy( m_storage->data(), s, count );
			m_storage->data()[ count ] = '\0';
		}
		else
		{
			*this = basic_ptr_string( s, count );
		}
		return *this;
	}

	basic_ptr_string& assign( const CharT* s )
	{
		return *this = s;
	}

	template <typename InputIt>
	basic_ptr_string& assign( InputIt first, const InputIt last )
	{
		const auto n = stdx::narrow_cast<size_type>( std::distance( first, last ) );
		if ( m_storage && m_storage->capacity >= n )
		{
			m_storage->size = n;
			auto dest = m_storage->data();
			for ( ; first != last; ++dest, ++first )
				Traits::assign( *dest, *first );

			*dest = '\0';
		}
		else
		{
			*this = basic_ptr_string( first, last );
		}
		return *this;
	}

	basic_ptr_string& assign( std::initializer_list<CharT> init )
	{
		return *this = init;
	}

	template <typename T>
	basic_ptr_string& assign( const T& t )
	{
		return *this = t;
	}

	template <typename T>
	basic_ptr_string& assign( const T& t, size_type pos, size_type count = npos )
	{
		const auto n = std::min( std::size( t ) - pos, count );
		if ( m_storage && m_storage->capacity >= n )
		{
			m_storage->size = n;
			auto dest = m_storage->data();
			const auto last = dest + n;
			for ( auto src = std::begin( t ); dest != last; ++dest, ++src )
				Traits::assign( *dest, *src );

			*dest = '\0';
		}
		else
		{
			*this = basic_ptr_string( t, pos, count );
		}
		return *this;
	}

	reference at( size_type index )
	{
		if ( index >= size() )
			throw std::out_of_range();

		return m_storage->data()[ index ];
	}

	const_reference at( size_type index ) const
	{
		if ( index >= size() )
			throw std::out_of_range();

		return m_storage->data()[ index ];
	}

	reference operator[]( size_type index ) noexcept
	{
		dbAssert( index < size() );
		return m_storage->data()[ index ];
	}

	const_reference operator[]( size_type index ) const noexcept
	{
		dbAssert( index < size() );
		return m_storage->data()[ index ];
	}

	reference front() noexcept
	{
		dbAssert( !empty() );
		return m_storage->data()[ 0 ];
	}

	const_reference front() const noexcept
	{
		dbAssert( !empty() );
		return m_storage->data()[ 0 ];
	}

	reference back() noexcept
	{
		dbAssert( !empty() );
		return m_storage->data()[ m_storage->size - 1 ];
	}

	const_reference back() const noexcept
	{
		dbAssert( !empty() );
		return m_storage->data()[ m_storage->size - 1 ];
	}

	pointer data() noexcept
	{
		return m_storage ? m_storage->data() : reinterpret_cast<pointer>( &m_storage );
	}

	const_pointer data() const noexcept
	{
		return m_storage ? m_storage->data() : reinterpret_cast<const_pointer>( &m_storage );
	}

	pointer c_str() noexcept
	{
		return m_storage ? m_storage->data() : reinterpret_cast<pointer>( &m_storage );
	}

	const_pointer c_str() const noexcept
	{
		return m_storage ? m_storage->data() : reinterpret_cast<const_pointer>( &m_storage );
	}

	operator view_type() const noexcept
	{
		return m_storage ? { m_storage->data(), m_storage->size } : std::basic_string_view<CharT>{};
	}

	// iterators

	iterator begin() noexcept { return m_storage ? m_storage->data() : nullptr; }
	iterator end() noexcept { return m_storage ? m_storage->end() : nullptr; }
	const_iterator begin() const noexcept { return m_storage ? m_storage->data() : nullptr; }
	const_iterator end() const noexcept { return m_storage ? m_storage->end() : nullptr; }
	const_iterator cbegin() const noexcept { return begin(); }
	const_iterator cend() const noexcept { return end(); }
	reverse_iterator rbegin() noexcept { return end(); }
	reverse_iterator rend() noexcept { return begin(); }
	const_reverse_iterator rbegin() const noexcept { return end(); }
	const_reverse_iterator rend() const noexcept { return begin(); }
	const_reverse_iterator crbegin() const noexcept { return end(); }
	const_reverse_iterator crend() const noexcept { return begin(); }

	// capacity

	[[nodiscard]] bool empty() const noexcept
	{
		return !m_storage || ( m_storage->size == 0 );
	}

	size_type size() const noexcept
	{
		return m_storage ? m_storage->size : 0;
	}

	size_type length() const noexcept
	{
		return size();
	}

	size_type max_size() const noexcept
	{
		return std::numeric_limits<size_type>::max();
	}

	void reserve( size_type n )
	{
		if ( !m_storage )
			m_storage = storage::create( n );
		else if ( m_storage->capacity < n )
			allocate_and_move( n );
	}

	size_type capacity() const noexcept
	{
		return m_storage ? m_storage->capacity : 0;
	}

	void shrink_to_fit()
	{
		if ( m_storage && m_storage->size > m_storage->capacity )
			allocate_and_move( m_storage->size );
	}

	// operations

	void clear()
	{
		if ( m_storage )
		{
			m_storage->size = 0;
			m_storage->data()[ 0 ] = '\0';
		}
	}

	basic_ptr_string& insert( size_type index, size_type count, CharT c )
	{
		dbAssert( index <= size() );
		reserve_extra( count );
		auto first = m_storage->data() + index;
		shift_right( first, count );
		Traits::assign( first, count, c );
		m_storage->size += count;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& insert( size_type index, const CharT* s )
	{
		dbAssert( index <= size() );
		const auto n = Traits::length( s );
		reserve_extra( n );
		auto first = m_storage->data() + index;
		shift_right( first, n );
		Traits::copy( first, s, n );
		m_storage->size += n;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& insert( size_type index, const CharT* s, size_type count )
	{
		dbAssert( index <= size() );
		reserve_extra( count );
		auto first = m_storage->data() + index;
		shift_right( first, count );
		Traits::copy( first, s, count );
		m_storage->size += count;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& insert( size_type index, const basic_ptr_string& str )
	{
		dbAssert( index <= size() );
		if ( !str.m_storage )
			return *this;

		reserve_extra( str.m_storage->size );
		auto first = m_storage->data() + index;
		shift_right( first, str.m_storage->size );
		Traits::copy( first, str.m_storage->data(), str.m_storage->size );
		m_storage->size += str.m_storage->size;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& insert( size_type index, const basic_ptr_string& str, size_type index_str, size_type count = npos )
	{
		dbAssert( index <= size() );
		dbAssert( index_str <= str.size() );

		if ( !str.m_storage )
			return *this;

		const auto n = std::min( str.m_storage->size - index_str, count );
		reserve_extra( n );
		auto first = m_storage->data() + index;
		shift_right( first, n );
		Traits::copy( first, str.m_storage->data() + index_str, n );
		m_storage->size += n;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& insert( const_iterator pos, CharT c )
	{
		return insert( stdx::narrow_cast<size_type>( pos - begin() ), 1, c );
	}

	basic_ptr_string& insert( const_iterator pos, size_type count, CharT c )
	{
		return insert( stdx::narrow_cast<size_type>( pos - begin() ), count, c );
	}

	template <typename InputIt>
	basic_ptr_string& insert( const_iterator pos, InputIt first, InputIt last )
	{
		const auto index = stdx::narrow_cast<size_type>( pos - begin() );
		dbAssert( index <= size() );
		const auto n = stdx::narrow_cast<size_type>( std::distance( first, last ) );
		reserve_extra( n );
		auto dest = m_storage->data() + index;
		shift_right( dest, n );
		for ( ; first != last; ++first, ++dest )
			Traits::assign( *dest, *first );

		m_storage->size += n;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& insert( const_iterator pos, std::initializer_list<CharT> init )
	{
		const auto index = stdx::narrow_cast<size_type>( pos - begin() );
		return insert( index, init.begin(), 0, init.size() );
	}

	template <typename T>
	basic_ptr_string& insert( size_type index, const T& t )
	{
		dbAssert( index <= size() );
		const auto n = std::size( t );
		reserve_extra( n );
		auto dest = m_storage->data() + index;
		shift_right( dest, n );
		const auto last = dest + n;
		for ( auto src = std::begin( t ); dest != last; ++dest, ++src )
			Traits::assign( *dest, *src );

		m_storage->size += n;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	template <typename T>
	basic_ptr_string& insert( size_type index, const T& t, size_type index_str, size_type count = npos )
	{
		dbAssert( index <= size() );
		dbAssert( index_str <= std::size( t ) );
		const auto n = std::min( std::size( t ) - index_str, count );
		reserve_extra( n );
		auto dest = m_storage->data() + index;
		shift_right( dest, n );
		const auto last = dest + n;
		for ( auto src = std::begin( t ) + index_str; dest != last; ++dest, ++src )
			Traits::assign( *dest, *src );

		m_storage->size += n;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& erase( size_type index = 0, size_type count = npos )
	{
		dbAssert( index <= size() );
		if ( m_storage )
		{
			const auto n = std::min( m_storage->size - index, count );
			shift_left( m_storage->data() + index + n, n );
			m_storage->size -= n;
			dbAssert( *m_storage->end() == '\0' );
		}
		return *this;
	}

	iterator erase( const_iterator pos )
	{
		dbAssert( m_storage );
		dbAssert( begin() <= pos );
		dbAssert( pos < end() );
		shift_left( const_cast<pointer>( pos ) + 1, 1 );
		m_storage->size -= 1;
		dbAssert( *m_storage->end() == '\0' );
		return first;
	}

	iterator erase( const_iterator first, const_iterator last )
	{
		dbAssert( m_storage );
		dbAssert( begin() <= first );
		dbAssert( first <= last );
		dbAssert( last < end() );
		const auto n = static_cast<size_type>( last - first );
		shift_left( const_cast<pointer>( last ), n );
		m_storage->size -= n;
		dbAssert( *m_storage->end() == '\0' );
		return const_cast<iterator>( first );
	}

	void push_back( CharT c )
	{
		reserve_extra( 1 );
		Traits::assign( m_storage->data()[ m_storage->size++ ], c );
		m_storage->data()[ m_storage->size ] = '\0';
	}

	void pop_back()
	{
		dbAssert( !empty() );
		m_storage->data()[ --m_storage->size ] = '\0';
	}

	basic_ptr_string& append( size_type count, CharT c )
	{
		reserve_extra( count );
		auto it = m_storage->end();
		it = Traits::assign( it, count, c );
		*it = '\0';
		return *this;
	}

	basic_ptr_string& append( const basic_ptr_string& str )
	{
		if ( str.m_storage )
		{
			reserve_extra( str.m_storage->size );
			auto it = m_storage->end();
			Traits::copy( it, str.m_storage->data(), str.m_storage->size + 1 );
			m_storage->size += str.m_storage->size;
			dbAssert( *m_storage->end() == '\0' );
		}
		return *this;
	}

	basic_ptr_string& append( const basic_ptr_string& str, size_type pos, size_type count = npos )
	{
		dbAssert( pos <= str.size() );
		if ( str.m_storage )
		{
			const auto n = std::min( str.m_storage->size - pos, count );
			reserve_extra( n );
			auto it = m_storage->end();
			it = Traits::copy( it, str.m_storage->data(), n );
			*it = '\0';
			m_storage->size += n;
			dbAssert( *m_storage->end() == '\0' );
		}
		return *this;
	}

	basic_ptr_string& append( const CharT* s, size_type count )
	{
		reserve_extra( count );
		auto last = Traits::copy( m_storage->end(), s, count );
		*last = '\0';
		m_storage->size += count;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& append( const CharT* s )
	{
		const auto n = Traits::length( s );
		reserve_extra( n );
		Traits::copy( m_storage->end(), s, n + 1 );
		m_storage->size += n;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	template <typename InputIt>
	basic_ptr_string& append( InputIt first, InputIt last )
	{
		const auto n = stdx::narrow_cast<size_type>( std::distance( first, last ) );
		reserve_extra( n );
		auto it = m_storage->end();
		for ( ; first != last; ++it, ++first )
			Traits::assign( *it, *first );

		*it = '\0';
		m_storage->size += n;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& append( std::initializer_list<CharT> init )
	{
		return append( init.begin(), init.size() );
	}

	template <typename T>
	basic_ptr_string& append( const T& t )
	{
		return append( std::begin( t ), std::end( t ) );
	}

	template <typename T>
	basic_ptr_string& append( const T& t, size_type pos, size_type count = npos )
	{
		dbAssert( pos <= std::size( t ) );
		const auto n = std::min( std::size( t ) - pos, count );
		auto dest = m_storage->end();
		auto last = dest + n;
		for ( auto src = std::begin( t ) + pos; dest != last; ++dest, ++src )
			Traits::assign( *dest, *src );

		*dest = '\0';
		m_storage->size += n;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& operator+=( const basic_ptr_string& str )
	{
		return append( str );
	}

	basic_ptr_string& operator+=( CharT c )
	{
		push_back( c );
		return *this;
	}

	basic_ptr_string& operator+=( const CharT* s )
	{
		return append( s );
	}

	basic_ptr_string& operator+=( std::initializer_list<CharT> init )
	{
		return append( init );
	}

	template <typename T>
	basic_ptr_string& operator+=( const T& t )
	{
		return append( t );
	}

	int compare( const basic_ptr_string& str ) const noexcept
	{
		return view_type{ *this }.compare( str );
	}

	int compare( size_type pos1, size_type count1, const basic_ptr_string& str ) const noexcept
	{
		return view_type{ *this }.compare( pos1, count1, str );
	}

	int compare( size_type pos1, size_type count1, const basic_ptr_string& str, size_type pos2, size_type count2 ) const noexcept
	{
		return view_type{ *this }.compare( pos1, count1, str, pos2, count2 );
	}

	int compare( const CharT* s ) const noexcept
	{
		return view_type{ *this }.compare( s );
	}

	int compare( size_type pos1, size_type count1, const CharT* s ) const noexcept
	{
		return view_type{ *this }.compare( pos1, count1, s );
	}

	int compare( size_type pos1, size_type count1, const CharT* s, size_type count2 ) const noexcept
	{
		return view_type{ *this }.compare( pos1, count1, s, count2 );
	}

	template <typename T>
	int compare( const T& t ) const noexcept
	{
		return view_type{ *this }.compare( t );
	}

	template <typename T>
	int compare( size_type pos1, size_type count1, const T& t ) const noexcept
	{
		return view_type{ *this }.compare( pos1, count1, t );
	}

	template <typename T>
	int compare( size_type pos1, size_type count1, const T& t, size_type pos2, size_type count2 ) const noexcept
	{
		return view_type{ *this }.compare( pos1, count1, t, pos2, count2 );
	}

	bool starts_with( view_type sv ) const noexcept
	{
		return view_type{ *this }.substr( 0, sv.size() ) == sv;
	}

	bool starts_with( CharT c ) const noexcept
	{
		return m_storage && Traits::eq( m_storage->data()[ 0 ] == c );
	}

	bool starts_with( const CharT* s ) const noexcept
	{
		return starts_with( view_type{ s } );
	}

	bool ends_with( view_type sv ) const noexcept
	{
		view_type str{ *this };
		return ( str.size() >= sv.size() ) ? str.substr( str.size() - sv.size() ) == sv : false;
	}

	bool ends_with( CharT c ) const noexcept
	{
		return m_storage && Traits::eq( m_storage->data()[ m_storage->size - 1 ] == c );
	}

	bool ends_with( const CharT* s ) const noexcept
	{
		return ends_with( view_type{ s } );
	}

	basic_ptr_string& replace( size_type pos, size_type count, const basic_ptr_string& str )
	{
		const auto diff = str.ssize() - static_cast<difference_type>( count );
		if ( diff > 0 )
			reserve_extra( static_cast<size_type>( diff ) );

		auto first = m_storage->data() + pos;
		auto last = first + count;
		shift( last, diff );
		Traits::copy( *first, str.data(), str.size() );
		m_storage->size += diff;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& replace( const_iterator first, const_iterator last, const basic_ptr_string& str )
	{
		dbAssert( begin() <= first );
		dbAssert( first <= last );
		dbAssert( last <= end() );
		return replace( static_cast<size_type>( first - begin() ), static_cast<size_type>( last - first ) );
	}

	basic_ptr_string& replace( size_type pos, size_type count, const basic_ptr_string& str, size_type pos2, size_type count2 = npos )
	{
		dbAssert( pos2 <= str.size() );
		const auto n = std::min( str.size() - pos2, count );

		const auto diff = static_cast<difference_type>( n ) - static_cast<difference_type>( count );
		if ( diff > 0 )
			reserve_extra( static_cast<size_type>( diff ) );

		auto first = m_storage->data() + pos;
		shift( first + count, diff );
		Traits::copy( *first, str.data() + pos2, n );
		m_storage->size += diff;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	template <typename InputIt>
	basic_ptr_string& replace( size_type pos, size_type count, InputIt first2, InputIt last2 )
	{
		dbAssert( pos <= size() );
		const auto diff = std::distance( first2, last2 ) - static_cast<difference_type>( count );
		if ( diff > 0 )
			reserve_extra( static_cast<size_type>( diff ) );

		auto dest = m_storage->data() + pos;
		shift( dest + count, diff );
		for ( ; first2 != last2; ++dest, ++first2 )
			Traits::assign( *dest, *first2 );

		m_storage->size += diff;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	template <typename InputIt>
	basic_ptr_string& replace( const_iterator first, const_iterator last, InputIt first2, InputIt last2 )
	{
		dbAssert( begin() <= first );
		dbAssert( first <= last );
		dbAssert( last <= end() );
		return replace( static_cast<size_type>( first - begin() ), static_cast<size_type>( last - first ), first2, last2 );
	}

	basic_ptr_string& replace( size_type pos, size_type count, const CharT* s, size_type count2 )
	{
		const auto diff = count2 - count;
		if ( diff > 0 )
			reserve_extra( static_cast<size_type>( diff ) );

		auto dest = m_storage->data() + pos;
		shift( dest + count, diff );
		Traits::copy( *dest, s, count2 );

		m_storage->size += diff;
		dbAssert( *m_storage->end() == '\0' );
		return *this;
	}

	basic_ptr_string& replace( size_type pos, size_type count, const CharT* s )
	{
		return replace( pos, count, s, Traits::length( s ) );
	}

	basic_ptr_string& replace( const_iterator first, const_iterator last, const CharT* s, size_type count2 )
	{
		dbAssert( begin() <= first );
		dbAssert( first <= last );
		dbAssert( last <= end() );
		return replace( static_cast<size_type>( first - begin() ), static_cast<size_type>( last - first ), s, count2 );
	}

	basic_ptr_string& replace( const_iterator first, const_iterator last, const CharT* s )
	{
		dbAssert( begin() <= first );
		dbAssert( first <= last );
		dbAssert( last <= end() );
		return replace( static_cast<size_type>( first - begin() ), static_cast<size_type>( last - first ), s, Traits::length( s ) );
	}

	template <typename T>
	basic_ptr_string& replace( size_type pos, size_type count, const T& t )
	{
		return replace( pos, count, std::begin( t ), std::end( t ) );
	}

	template <typename T>
	basic_ptr_string& replace( size_type pos, size_type count, const T& t, size_type pos2, size_type count2 = npos )
	{
		dbAssert( pos <= size() );
		dbAssert( pos2 <= std::size( t ) );
		const auto n = std::min( std::size( t ) - pos2, count2 );
		const auto first = std::begin( t ) + pos2;
		return replace( pos, count, first, first + n );
	}

	template <typename T>
	basic_ptr_string& replace( const_iterator first, const_iterator last, const T& t )
	{
		return replace( first, last, std::begin( t ), std::end( t ) );
	}

	basic_ptr_string substr( size_type pos = 0, size_type count = npos ) const
	{
		return basic_ptr_string( *this, pos, count );
	}

	size_type copy( CharT* dest, size_type count, size_type pos = 0 ) const
	{
		dbAssert( pos <= size() );
		if ( pos > size() )
			throw std::out_of_range();

		const auto src = data() + pos;
		const auto n = std::min( size() - pos, count );
		Traits::copy( dest, src, n );
		return n;
	}

	void resize( size_type count )
	{
		resize( count, CharT() );
	}

	void resize( size_type count, CharT c )
	{
		if ( count > size() )
		{
			reserve( count );
			const auto first = m_storage->end();
			const auto last = m_storage->data() + count;
			Traits::assign( first, static_cast<size_type>( last - first ), c );
			m_storage->size = count;
			*last = '\0';
		}
		else if ( m_storage )
		{
			m_storage->size = count;
			*m_storage->end() = '\0';
		}
	}

	void swap( basic_ptr_string& other ) noexcept
	{
		m_storage = std::exchange( other.m_storage, m_storage );
	}

	// search

	size_type find( const basic_ptr_string& str, size_type pos = 0 ) const noexcept
	{
		return view_type{ *this }.find( str, pos );
	}

	size_type find( const CharT* s, size_type pos, size_type count ) const
	{
		return view_type{ *this }.find( s, pos, count );
	}

	size_type find( const CharT* s, size_type pos = 0 ) const
	{
		return view_type{ *this }.find( s, pos );
	}

	size_type find( CharT c, size_type pos = 0) const noexcept
	{
		return view_type{ *this }.find( c, pos );
	}

	template <typename T>
	size_type find( const T& t, size_type pos = 0 ) const noexcept
	{
		return view_type{ *this }.find( t, pos );
	}

	size_type rfind( const basic_ptr_string& str, size_type pos = npos ) const noexcept
	{
		return view_type{ *this }.rfind( str, pos );
	}

	size_type rfind( const CharT* s, size_type pos, size_type count ) const
	{
		return view_type{ *this }.rfind( s, pos, count );
	}

	size_type rfind( const CharT* s, size_type pos = npos ) const
	{
		return view_type{ *this }.rfind( s, pos );
	}

	size_type rfind( CharT c, size_type pos = npos ) const noexcept
	{
		return view_type{ *this }.rfind( c, pos );
	}

	template <typename T>
	size_type rfind( const T& t, size_type pos = npos ) const noexcept
	{
		return view_type{ *this }.find_first_of( t, pos );
	}

	size_type find_first_of( const basic_ptr_string& str, size_type pos = 0 ) const noexcept
	{
		return view_type{ *this }.find_first_of( str, pos );
	}

	size_type find_first_of( const CharT* s, size_type pos, size_type count ) const
	{
		return view_type{ *this }.find_first_of( s, pos, count );
	}

	size_type find_first_of( const CharT* s, size_type pos = 0 ) const
	{
		return view_type{ *this }.find_first_of( s, pos );
	}

	size_type find_first_of( CharT c, size_type pos = 0 ) const noexcept
	{
		return view_type{ *this }.find_first_of( c, pos );
	}

	template <typename T>
	size_type find_first_of( const T& t, size_type pos = 0 ) const noexcept
	{
		return view_type{ *this }.find_first_of( t, pos );
	}

	size_type find_first_not_of( const basic_ptr_string& str, size_type pos = 0 ) const noexcept
	{
		return view_type{ *this }.find_first_not_of( str, pos );
	}

	size_type find_first_not_of( const CharT* s, size_type pos, size_type count ) const
	{
		return view_type{ *this }.find_first_not_of( s, pos, count );
	}

	size_type find_first_not_of( const CharT* s, size_type pos = 0 ) const
	{
		return view_type{ *this }.find_first_not_of( s, pos );
	}

	size_type find_first_not_of( CharT c, size_type pos = 0 ) const noexcept
	{
		return view_type{ *this }.find_first_not_of( c, pos );
	}

	template <typename T>
	size_type find_first_not_of( const T& t, size_type pos = 0 ) const noexcept
	{
		return view_type{ *this }.find_first_not_of( t, pos );
	}

	size_type find_last_of( const basic_ptr_string& str, size_type pos = npos ) const noexcept
	{
		return view_type{ *this }.find_last_of( str, pos );
	}

	size_type find_last_of( const CharT* s, size_type pos, size_type count ) const
	{
		return view_type{ *this }.find_last_of( s, pos, count );
	}

	size_type find_last_of( const CharT* s, size_type pos = npos ) const
	{
		return view_type{ *this }.find_last_of( s, pos );
	}

	size_type find_last_of( CharT c, size_type pos = npos ) const noexcept
	{
		return view_type{ *this }.find_last_of( c, pos );
	}

	template <typename T>
	size_type find_last_of( const T& t, size_type pos = npos ) const noexcept
	{
		return view_type{ *this }.find_last_of( t, pos );
	}

	size_type find_last_not_of( const basic_ptr_string& str, size_type pos = npos ) const noexcept
	{
		return view_type{ *this }.find_last_not_of( str, pos );
	}

	size_type find_last_not_of( const CharT* s, size_type pos, size_type count ) const
	{
		return view_type{ *this }.find_last_not_of( s, pos, count );
	}

	size_type find_last_not_of( const CharT* s, size_type pos = npos ) const
	{
		return view_type{ *this }.find_last_not_of( s, pos );
	}

	size_type find_last_not_of( CharT c, size_type pos = npos ) const noexcept
	{
		return view_type{ *this }.find_last_not_of( c, pos );
	}

	template <typename T>
	size_type find_last_not_of( const T& t, size_type pos = npos ) const noexcept
	{
		return view_type{ *this }.find_last_not_of( t, pos );
	}

	// non member functions

	friend basic_ptr_string operator+( const basic_ptr_string& lhs, const basic_ptr_string& rhs )
	{
		basic_ptr_string result;
		result.reserve( lhs.size() + rhs.size() );
		return ( result += lhs ) += rhs;
	}

	friend basic_ptr_string operator+( const basic_ptr_string& lhs, const CharT* rhs )
	{
		basic_ptr_string result;
		result.reserve( lhs.size() + std::size( rhs ) );
		return ( result += lhs ) += rhs;
	}

	friend basic_ptr_string operator+( const basic_ptr_string& lhs, CharT rhs )
	{
		basic_ptr_string result;
		result.reserve( lhs.size() + 1 );
		return ( result += lhs ) += rhs;
	}

	friend basic_ptr_string operator+( const CharT* lhs, const basic_ptr_string& rhs )
	{
		basic_ptr_string result;
		result.reserve( std::size( lhs ) + rhs.size() );
		return ( result += lhs ) += rhs;
	}

	friend basic_ptr_string operator+( CharT lhs, const basic_ptr_string& rhs )
	{
		basic_ptr_string result;
		result.reserve( 1 + rhs.size() );
		return ( result += lhs ) += rhs;
	}

	friend basic_ptr_string operator+( basic_ptr_string&& lhs, basic_ptr_string&& rhs )
	{
		return std::move( lhs += rhs );
	}

	friend basic_ptr_string operator+( basic_ptr_string&& lhs, const basic_ptr_string& rhs )
	{
		return std::move( lhs += rhs );
	}

	friend basic_ptr_string operator+( basic_ptr_string&& lhs, const CharT* rhs )
	{
		return std::move( lhs += rhs );
	}

	friend basic_ptr_string operator+( basic_ptr_string&& lhs, CharT rhs )
	{
		return std::move( lhs += rhs );
	}

	friend basic_ptr_string operator+( const basic_ptr_string& lhs, basic_ptr_string&& rhs )
	{
		rhs.insert( rhs.begin(), lhs );
		return std::move( rhs );
	}

	friend basic_ptr_string operator+( const CharT* lhs, basic_ptr_string&& rhs )
	{
		rhs.insert( rhs.begin(), lhs );
		return std::move( rhs );
	}

	friend basic_ptr_string operator+( CharT lhs, basic_ptr_string&& rhs )
	{
		rhs.insert( rhs.begin(), lhs );
		return std::move( rhs );
	}

	friend bool operator==( const basic_ptr_string& lhs, const basic_ptr_string& rhs ) noexcept
	{
		return view_type{ lhs } == view_type{ rhs };
	}

	friend bool operator!=( const basic_ptr_string& lhs, const basic_ptr_string& rhs ) noexcept
	{
		return view_type{ lhs } != view_type{ rhs };
	}

	friend bool operator<( const basic_ptr_string& lhs, const basic_ptr_string& rhs ) noexcept
	{
		return view_type{ lhs } < view_type{ rhs };
	}

	friend bool operator>( const basic_ptr_string& lhs, const basic_ptr_string& rhs ) noexcept
	{
		return view_type{ lhs } > view_type{ rhs };
	}

	friend bool operator<=( const basic_ptr_string& lhs, const basic_ptr_string& rhs ) noexcept
	{
		return view_type{ lhs } <= view_type{ rhs };
	}

	friend bool operator>=( const basic_ptr_string& lhs, const basic_ptr_string& rhs ) noexcept
	{
		return view_type{ lhs } >= view_type{ rhs };
	}

	friend bool operator==( const basic_ptr_string& lhs, const CharT* rhs )
	{
		return view_type{ lhs } == rhs;
	}

	friend bool operator!=( const basic_ptr_string& lhs, const CharT* rhs )
	{
		return view_type{ lhs } != rhs;
	}

	friend bool operator<( const basic_ptr_string& lhs, const CharT* rhs )
	{
		return view_type{ lhs } < rhs;
	}

	friend bool operator>( const basic_ptr_string& lhs, const CharT* rhs )
	{
		return view_type{ lhs } > rhs;
	}

	friend bool operator<=( const basic_ptr_string& lhs, const CharT* rhs )
	{
		return view_type{ lhs } <= rhs;
	}

	friend bool operator>=( const basic_ptr_string& lhs, const CharT* rhs )
	{
		return view_type{ lhs } >= rhs;
	}

	friend bool operator==( const CharT* lhs, const basic_ptr_string& rhs )
	{
		return lhs == view_type{ rhs };
	}

	friend bool operator!=( const CharT* lhs, const basic_ptr_string& rhs )
	{
		return lhs != view_type{ rhs };
	}

	friend bool operator<( const CharT* lhs, const basic_ptr_string& rhs )
	{
		return lhs < view_type{ rhs };
	}

	friend bool operator>( const CharT* lhs, const basic_ptr_string& rhs )
	{
		return lhs > view_type{ rhs };
	}

	friend bool operator<=( const CharT* lhs, const basic_ptr_string& rhs )
	{
		return lhs <= view_type{ rhs };
	}

	friend bool operator>=( const CharT* lhs, const basic_ptr_string& rhs )
	{
		return lhs >= view_type{ rhs };
	}

private:
	void allocate_and_move( size_type n )
	{
		dbAssert( m_storage );
		auto* newStorage = storage::create( n, m_storage->size );
		Traits::copy( newStorage->data(), static_cast<const CharT*>( m_storage->data() ), m_storage->size + 1 );
		delete m_storage;
		m_storage = newStorage;
	}

	void reserve_extra( size_type n )
	{
		if ( !m_storage )
			m_storage = storage::create( std::max( n, 16 ) );
		else if ( m_storage->capacity < m_storage->size + n )
			allocate_and_move( std::max( m_storage->size + n, m_storage->capacity * 2 ) );
	}

	void shift( pointer pos, difference_type distance )
	{
		Traits::move(
			pos + distance,
			static_cast<const_pointer>( pos ),
			static_cast<size_type>( m_storage->end() - pos + 1 ) );
	}

	void shift_right( pointer pos, size_type count )
	{
		shift( pos, count );
	}

	void shift_left( pointer pos, size_type count )
	{
		shift( pos, -static_cast<difference_type>( count ) );
	}

private:
	struct storage
	{
		size_type size;
		size_type capacity;

		CharT* data() noexcept { return reinterpret_cast<CharT*>( this + sizeof( storage ) ); }
		const CharT* data() noexcept { return reinterpret_cast<const CharT*>( this + sizeof( storage ) ); }

		CharT* end() noexcept { return data() + size; }
		const CharT* end() const noexcept { return data() + size; }

		static create( size_type capacity, size_type size = 0 )
		{
			dbAssert( capacity > 0 );
			auto* s = reinterpret_cast<storage*>( new[ sizeof( storage ) + ( capacity_ + 1 ) * sizeof( CharT ) ] );
			s->capacity = capacity;
			s->size = size;
			s->data()[ size ] = '\0';
			return s;
		}

		void* operator new( size_t ) = delete;

		void operator delete( void* p )
		{
			::delete[] static_cast<char*>( p );
		}
	};

	storage* m_storage = nullptr;
};


using ptr_string = basic_ptr_string<char>;
using wptr_string = basic_ptr_string<wchar_t>;
using u16ptr_string = basic_ptr_string<char16_t>;
using u32ptr_string = basic_ptr_string<char32_t>;

static_assert( sizeof( ptr_string ) == sizeof( void* ) );
static_assert( sizeof( wptr_string ) == sizeof( void* ) );
static_assert( sizeof( u16ptr_string ) == sizeof( void* ) );
static_assert( sizeof( u32ptr_string ) == sizeof( void* ) );

}

namespace std
{
	template <typename CharT, typename Traits>
	struct hash<stdx::basic_ptr_string<CharT, Traits>>
	{
		std::size_t operator()( const stdx::basic_ptr_string<CharT, Traits>& str ) const noexcept
		{
			return std::hash<std::basic_string_view<CharT, Traits>>{}( str );
		}
	};
}