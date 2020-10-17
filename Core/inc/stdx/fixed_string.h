#ifndef STDX_FIXED_STRING_HPP
#define STDX_FIXED_STRING_HPP

#include <cassert>
#include <string_view>

#define dbExpects( condition ) do { const int i = ( condition ) ? 0 : ( assert( false ), 0 ); } while( false )

namespace stdx {

template <typename CharT, std::size_t N, typename Traits = std::char_traits<CharT>>
class basic_fixed_string
{
public:
	using traits_type = Traits;
	using value_type = CharT;
	using view = std::basic_string_view<CharT, Traits>;
	using size_type = typename view::size_type;
	using difference_type = typename view::difference_type;
	using pointer = CharT*;
	using const_pointer = const CharT*;
	using reference = CharT&;
	using const_reference = const CharT&;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static constexpr size_type npos = view::npos;
	static constexpr CharT null_terminator = CharT();

	// construction / assignment

	constexpr basic_fixed_string() noexcept = default;
	template <std::size_t N2>
	constexpr basic_fixed_string( const basic_fixed_string<CharT, N2, Traits>& other ) noexcept( N2 <= N ) { append( other.begin(), other.end() ); }
	constexpr basic_fixed_string( view str ) { append( view{ str } ); }
	constexpr basic_fixed_string( const_pointer str ) { append( str ); }
	constexpr basic_fixed_string( std::initializer_list<CharT> init ) { append( init ); }

	template <std::size_t N2>
	constexpr basic_fixed_string( const CharT( &str )[ N2 ] ) noexcept { append( str, N2 - 1 ); }
	
	constexpr basic_fixed_string& operator=( const basic_fixed_string& ) noexcept = default;
	template <std::size_t N2>
	constexpr basic_fixed_string& operator=( const CharT( &str )[ N2 ] ) noexcept( N2 <= N + 1 ) { return assign( str, N2 - 1 ); }
	constexpr basic_fixed_string& operator=( view str ) { return assign( str ); }
	constexpr basic_fixed_string& operator=( const_pointer str ) { return assign( str ); }

	constexpr basic_fixed_string& assign( size_type count, CharT c )
	{
		m_size = 0;
		return append( count, c );
	}
	constexpr basic_fixed_string& assign( view str )
	{
		m_size = 0;
		return append( str );
	}
	constexpr basic_fixed_string& assign( const_pointer str, size_type count )
	{
		m_size = 0;
		return append( str, count );
	}
	constexpr basic_fixed_string& assign( view str, size_type pos, size_type count = npos )
	{
		m_size = 0;
		return append( str, pos, count );
	}
	constexpr basic_fixed_string& assign( std::initializer_list<CharT> init )
	{
		m_size = 0;
		return append( init );
	}
	template <typename InputIt>
	constexpr basic_fixed_string& assign( InputIt first, InputIt last )
	{
		m_size = 0;
		return append( first, last );
	}

	// access

	constexpr reference at( size_type pos )
	{
		dbExpects( pos < size() );
		return m_data[ pos ];
	}
	constexpr const_reference at( size_type pos ) const
	{
		dbExpects( pos < size() + 1 );
		return m_data[ pos ];
	}

	constexpr reference operator[]( size_type pos ) noexcept
	{
		dbExpects( pos < size() );
		return m_data[ pos ];
	}
	constexpr const_reference operator[]( size_type pos ) const noexcept
	{
		dbExpects( pos < size() + 1 );
		return m_data[ pos ];
	}

	constexpr reference front() noexcept
	{
		dbExpects( !empty() );
		return m_data[ 0 ];
	}
	constexpr const_reference front() const noexcept
	{
		dbExpects( !empty() );
		return m_data[ 0 ];
	}

	constexpr reference back() noexcept
	{
		dbExpects( !empty() );
		return *( end() - 1 );
	}
	constexpr const_reference back() const noexcept 
	{
		dbExpects( !empty() );
		return *( end() - 1 );
	}

	constexpr pointer data() noexcept { return m_data; }
	constexpr const_pointer data() const noexcept { return m_data; }
	constexpr const_pointer c_str() const noexcept { return m_data; }

	constexpr operator view() const noexcept { return view{ data(), size() }; }

	// iterators

	constexpr iterator begin() noexcept { return m_data; }
	constexpr iterator end() noexcept { return m_data + m_size; }

	constexpr const_iterator begin() const noexcept { return m_data; }
	constexpr const_iterator end() const noexcept { return m_data + m_size; }

	constexpr const_iterator cbegin() const noexcept { return begin(); }
	constexpr const_iterator cend() const noexcept { return end(); }

	constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{ end() }; }
	constexpr reverse_iterator rend() noexcept { return reverse_iterator{ begin() }; }

	constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{ end() }; }
	constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{ begin() }; }

	constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
	constexpr const_reverse_iterator crend() const noexcept { return rend(); }

	// capacity

	constexpr bool empty() const noexcept { return m_size == 0; }
	constexpr size_type size() const noexcept { return m_size; }
	constexpr difference_type ssize() const noexcept { return static_cast<difference_type>( m_size ); }
	constexpr size_type length() const noexcept { return m_size; }
	constexpr size_type max_size() const noexcept { return N; }
	constexpr size_type capacity() const noexcept { return N; }

	// operations

	constexpr void clear() noexcept
	{
		m_data[ 0 ] = null_terminator;
		m_size = 0;
	}

	constexpr basic_fixed_string& insert( size_type insert, size_type count, CharT c );
	//...

	constexpr basic_fixed_string& erase( size_type index, size_type count = npos );
	//...

	constexpr void push_back( CharT c )
	{
		dbExpects( size() < capacity() );
		Traits::assign( m_data[ m_size ], c );
		m_data[ ++m_size ] = null_terminator;
	}

	constexpr void pop_back()
	{
		dbExpects( !empty() );
		m_data[ --m_size ] = null_terminator;
	}

	constexpr basic_fixed_string& append( size_type count, CharT c )
	{
		dbExpects( m_size + count <= capacity() );
		auto dest = end();
		for( auto last = end() + count; dest != last; ++dest )
		{
			Traits::assign( *dest, c );
		}
		*dest = null_terminator;
		m_size += count;
		return *this;
	}

	constexpr basic_fixed_string& append( view str ) { return append( str.begin(), str.end() ); }

	constexpr basic_fixed_string& append( view str, size_type pos, size_type count = npos )
	{
		dbExpects( pos <= str.size() );
		return append( str.begin() + pos, str.begin() + std::min( pos + count, str.size() ) );
	}

	constexpr basic_fixed_string& append( const_pointer str, size_type count ) { return append( str, str + count ); }

	constexpr basic_fixed_string& append( std::initializer_list<CharT> init ) { return append( init.begin(), init.end() ); }

	template <typename InputIt>
	constexpr basic_fixed_string& append( InputIt first, InputIt last )
	{
		if constexpr ( std::is_integral_v<InputIt> )
		{
			return append( static_cast<size_type>( first ), static_cast<CharT>( last ) );
		}
		else
		{
			dbExpects( m_size + std::distance( first, last ) <= capacity() );
			auto dest = end();
			m_size += std::distance( first, last );
			for( ; first != last; ++first, ++dest )
			{
				Traits::assign( *dest, static_cast<CharT>( *first ) );
			}
			*dest = null_terminator;
			return *this;
		}
	}

	constexpr basic_fixed_string& operator+=( view str ) { return append( str ); }
	constexpr basic_fixed_string& operator+=( CharT c ) { push_back( c ); return *this; }
	template<std::size_t N2>
	constexpr basic_fixed_string& operator+=( const CharT( &str )[ N2 ] ) { return append( str, N2 - 1 ); }
	constexpr basic_fixed_string& operator+=( std::initializer_list<CharT> init ) { return append( init.begin(), init.end() ); }

	constexpr int compare( view str ) const noexcept { return view{ *this }.compare( str ); }
	constexpr int compare( size_type pos1, size_type count1, view str ) const { return view{ *this }.compare( pos1, count1, str ); }
	constexpr int compare( size_type pos1, size_type count1, view str, size_type pos2, size_type count2 ) const { return view{ *this }.compare( pos1, count1, str, pos2, count2 ); }

	constexpr int compare( const_pointer str ) const { return view{ *this }.compare( str ); }
	constexpr int compare( size_type pos1, size_type count1, const_pointer str ) const { return view{ *this }.compare( pos1, count1, str ); }
	constexpr int compare( size_type pos1, size_type count1, const_pointer str, size_type pos2, size_type count2 ) const { return view{ *this }.compare( pos1, count1, str, pos2, count2 ); }

	constexpr basic_fixed_string& replace( size_type pos, size_type count, view str )
	{
		dbExpects( pos <= size() );
		auto dest = m_data + pos;
		for( auto src = str.begin(), last = str.begin() + count; src != last; ++dest, ++src )
		{
			Traits::assign( *dest, *src );
		}
		m_size = std::min( m_size, pos + count );
		*end() = null_terminator;
	}
	//...

	constexpr view substr( size_type pos = 0, size_type count = npos ) const
	{
		dbExpects( pos <= size() );
		return view{ m_data + pos, std::min( size() - pos, count ) };
	}

	constexpr size_type copy( pointer dest, size_type count, size_type pos = 0 ) const;

	constexpr void resize( size_type count )
	{
		dbExpects( count <= capacity() );
		if ( count < size() )
		{
			m_size = count;
			*end() = null_terminator;
		}
		else
		{
			// should we increase size in this case?
			for( auto it = end() + 1, last = begin() + count + 1; it != last; ++it )
			{
				*it = null_terminator;
			}
			m_size = count;
		}
	}

	constexpr void resize( size_type count, CharT c )
	{
		dbExpects( count <= capacity() );
		if ( count < size() )
		{
			m_size = count;
			*end() = null_terminator;
		}
		else
		{
			auto it = end();
			for( auto last = begin() + count; it != last; ++it )
			{
				Traits::assign( *it, c );
			}
			*it = null_terminator;
			m_size = count;
		}
	}

	constexpr void swap( basic_fixed_string& other ) noexcept
	{
		std::swap( m_data, other.m_data );
		std::swap( m_size, other.m_size );
	}

	// search

	constexpr size_type find( view str, size_type pos = 0 ) const { return view{ *this }.find( str, pos ); }
	constexpr size_type find( view str, size_type pos, size_type count ) const { return view{ *this }.find( str, pos, count ); }
	constexpr size_type find( CharT c, size_type pos = 0 ) const noexcept { return view{ *this }.find( c, pos ); }

	constexpr size_type rfind( view str, size_type pos = npos ) const { return view{ *this }.rfind( str, pos ); }
	constexpr size_type rfind( view str, size_type pos, size_type count ) const { return view{ *this }.rfind( str, pos, count ); }
	constexpr size_type rfind( CharT c, size_type pos = npos ) const noexcept { return view{ *this }.rfind( c, pos ); }

	constexpr size_type find_first_of( view str, size_type pos = 0 ) const { return view{ *this }.find_first_of( str, pos ); }
	constexpr size_type find_first_of( view str, size_type pos, size_type count ) const { return view{ *this }.find_first_of( str, pos, count ); }
	constexpr size_type find_first_of( CharT c, size_type pos = 0 ) const noexcept { return view{ *this }.find_first_of( c, pos ); }

	constexpr size_type find_first_not_of( view str, size_type pos = 0 ) const { return view{ *this }.find_first_not_of( str, pos ); }
	constexpr size_type find_first_not_of( view str, size_type pos, size_type count ) const { return view{ *this }.find_first_not_of( str, pos, count ); }
	constexpr size_type find_first_not_of( CharT c, size_type pos = 0 ) const noexcept { return view{ *this }.find_first_not_of( c, pos ); }

	constexpr size_type find_last_of( view str, size_type pos = npos ) const { return view{ *this }.find_last_of( str, pos ); }
	constexpr size_type find_last_of( view str, size_type pos, size_type count ) const { return view{ *this }.find_last_of( str, pos, count ); }
	constexpr size_type find_last_of( CharT c, size_type pos = npos ) const noexcept { return view{ *this }.find_last_of( c, pos ); }

	constexpr size_type find_last_not_of( view str, size_type pos = npos ) const { return view{ *this }.find_last_not_of( str, pos ); }
	constexpr size_type find_last_not_of( view str, size_type pos, size_type count ) const { return view{ *this }.find_last_not_of( str, pos, count ); }
	constexpr size_type find_last_not_of( CharT c, size_type pos = npos ) const noexcept { return view{ *this }.find_last_not_of( c, pos ); }

private:
	CharT m_data[ N ]{};
	size_type m_size = 0;
};

template <typename CharT, std::size_t N>
basic_fixed_string( const CharT( &str )[ N ] ) noexcept -> basic_fixed_string<CharT, N - 1>;

template <typename CharT, std::size_t N1, std::size_t N2, typename Traits>
constexpr basic_fixed_string<CharT, N1 + N2, Traits> operator+( const basic_fixed_string<CharT, N1, Traits>& lhs, const basic_fixed_string<CharT, N2, Traits>& rhs )
{
	basic_fixed_string<CharT, N1 + N2, Traits> result( lhs );
	result += rhs;
	return result;
}

template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr basic_fixed_string<CharT, N, Traits> operator+( basic_fixed_string<CharT, N, Traits> lhs, const T& rhs )
{
	return lhs += rhs;
}

template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr basic_fixed_string<CharT, N, Traits> operator+( const T& lhs, basic_fixed_string<CharT, N, Traits> rhs )
{
	return basic_fixed_string<CharT, N, Traits>{ lhs } += rhs;
}

template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator==( const basic_fixed_string<CharT, N, Traits>& lhs, const T& rhs )
{
	return lhs.compare( rhs ) == 0;
}
template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator==( const T& lhs, const basic_fixed_string<CharT, N, Traits>& rhs )
{
	return rhs.compare( lhs ) == 0;
}

template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator!=( const basic_fixed_string<CharT, N, Traits>& lhs, const T& rhs )
{
	return !( lhs == rhs );
}
template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator!=( const T& lhs, const basic_fixed_string<CharT, N, Traits>& rhs )
{
	return !( lhs == rhs );
}

template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator<( const basic_fixed_string<CharT, N, Traits>& lhs, const T& rhs )
{
	return lhs.compare( rhs ) < 0;
}
template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator<( const T& lhs, const basic_fixed_string<CharT, N, Traits>& rhs )
{
	return rhs.compare( lhs ) > 0;
}

template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator>( const basic_fixed_string<CharT, N, Traits>& lhs, const T& rhs )
{
	return rhs < lhs;
}
template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator>( const T& lhs, const basic_fixed_string<CharT, N, Traits>& rhs )
{
	return rhs < lhs;
}

template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator<=( const basic_fixed_string<CharT, N, Traits>& lhs, const T& rhs )
{
	return !( lhs > rhs );
}
template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator<=( const T& lhs, const basic_fixed_string<CharT, N, Traits>& rhs )
{
	return !( lhs > rhs );
}

template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator>=( const basic_fixed_string<CharT, N, Traits>& lhs, const T& rhs )
{
	return !( lhs < rhs );
}
template <typename CharT, std::size_t N, typename Traits, typename T>
constexpr bool operator>=( const T& lhs, const basic_fixed_string<CharT, N, Traits>& rhs )
{
	return !( lhs < rhs );
}

template <std::size_t N> using fixed_string = basic_fixed_string<char, N>;
template <std::size_t N> using u8fixed_string = basic_fixed_string<char, N>;
template <std::size_t N> using u16fixed_string = basic_fixed_string<char16_t, N>;
template <std::size_t N> using u32fixed_string = basic_fixed_string<char32_t, N>;
template <std::size_t N> using wfixed_string = basic_fixed_string<wchar_t, N>;

constexpr fixed_string<32> Alpha = "abcdefghijklmnopqrstuvwxyz";
static_assert( Alpha.size() == 26 );
static_assert( Alpha.ssize() == 26 );
static_assert( Alpha.length() == 26 );
static_assert( Alpha.capacity() == 32 );
static_assert( Alpha.max_size() == 32 );
static_assert( !Alpha.empty() );
static_assert( Alpha[ 2 ] == 'c' );
static_assert( Alpha.front() == 'a' );
static_assert( Alpha.back() == 'z' );
static_assert( *Alpha.begin() == 'a' );
static_assert( *Alpha.end() == '\0' );
static_assert( *( Alpha.end() - 1 ) == 'z' );
static_assert( Alpha.data() == Alpha.c_str() );
static_assert( Alpha.data() == Alpha.begin() );
static_assert( Alpha.data() == &( Alpha[ 0 ] ) );
static_assert( Alpha == "abcdefghijklmnopqrstuvwxyz" );
static_assert( Alpha >= "abcdefghijklmnopqrstuvwxyz" );
static_assert( Alpha <= "abcdefghijklmnopqrstuvwxyz" );
static_assert( Alpha != "abcdefghijklmnopqrstuvwxy" );
static_assert( Alpha < "zz" );
static_assert( Alpha <= "zz" );
static_assert( Alpha > "aa" );
static_assert( Alpha >= "aa" );
static_assert( "abcdefghijklmnopqrstuvwxyz" == Alpha );
static_assert( "abcdefghijklmnopqrstuvwxyz" <= Alpha );
static_assert( "abcdefghijklmnopqrstuvwxyz" >= Alpha );
static_assert( "abcdefghijklmnopqrstuvwxy" != Alpha );
static_assert( "zz" > Alpha );
static_assert( "zz" >= Alpha );
static_assert( "aa" < Alpha );
static_assert( "aa" <= Alpha );

constexpr fixed_string<32> Empty{};
static_assert( Empty.empty() );
static_assert( Empty.size() == 0 );
static_assert( Empty == "" );
static_assert( Empty.begin() == Empty.end() );
static_assert( Empty[ 0 ] == '\0' );

constexpr fixed_string<32> CreateAppendedString()
{
	fixed_string<32> str = "123";
	str += "45";
	str += '6';
	str.append( "78" );
	str.push_back( '9' );
	str.append( 2, '0' );
	str.pop_back();
	return str;
}
constexpr fixed_string<32> Appended = CreateAppendedString();
static_assert( Appended.size() == 10 );
static_assert( *Appended.end() == '\0' );

constexpr auto CreateConcatString()
{
	constexpr basic_fixed_string str1 = "123";
	constexpr basic_fixed_string str2 = "4567890";
	return str1 + str2;
}
constexpr auto Concat = CreateConcatString();
static_assert( Concat.size() == 10 );
static_assert( Concat.capacity() == 10 );

} // namespace stdx

#endif