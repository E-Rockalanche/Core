#pragma once

#include <stdx/assert.h>

#include <string_view>

namespace stdx
{

template <class CharT, class Traits = std::char_traits<CharT>>
class basic_zstring_view
{
public:
	static_assert( std::is_same_v<CharT, Traits::char_type> );

	using view_type = std::basic_string_view<CharT, Traits>;

	using traits_type = Traits;
	using value_type = CharT;
	using pointer = CharT * ;
	using const_pointer = const CharT*;
	using reference = CharT & ;
	using const_reference = const CharT&;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using size_type = typename view_type::size_type;
	using difference_type = typename view_type::difference_type;

	static constexpr size_type = view_type::npos;

	constexpr basic_zstring_view() noexcept = default;

	constexpr basic_zstring_view( const basic_zstring_view& other ) noexcept = default;

	constexpr basic_zstring_view( const CharT* s, size_type count )
		: m_data{ s }
		, m_size{ count }
	{
		dbEnsures( m_data[ m_size ] == 0 );
	}

	constexpr basic_zstring_view( const CharT* s )
		: m_data{ s }
		, m_size{ Traits::length( s ) }
	{}

	template <typename It, typename End>
	constexpr basic_zstring_view( It first, End last )
		: m_data{ std::addressof( *first ) }
		, m_size{ static_cast<size_t>( last - first ) }
	{
		dbEnsures( m_data[ m_size ] == 0 );
	}

	constexpr basic_zstring_view& operator=( const basic_zstring_view& ) noexcept = default;

	constexpr operator view_type() const noexcept
	{
		return view_type{ m_data, m_size };
	}

	constexpr iterator begin() noexcept { return m_data; }
	constexpr iterator end() noexcept { return m_data + m_size; }

	constexpr const_iterator begin() const noexcept { return m_data; }
	constexpr const_iterator end() const noexcept { return m_data + m_size; }

	constexpr const_iterator cbegin() const noexcept { return m_data; }
	constexpr const_iterator cend() const noexcept { return m_data + m_size; }

	constexpr reverse_iterator rbegin() noexcept { return end(); }
	constexpr reverse_iterator rend() noexcept { return begin(); }

	constexpr const_reverse_iterator rbegin() const noexcept { return end(); }
	constexpr const_reverse_iterator rend() const noexcept { return begin(); }

	constexpr const_reverse_iterator crbegin() const noexcept { return end(); }
	constexpr const_reverse_iterator crend() const noexcept { return begin(); }

	constexpr const_reference operator[]( size_type pos ) const noexcept
	{
		dbExpects( pos < m_size + 1 );
		return m_data[ pos ];
	}

	constexpr const_reference at( size_type pos ) const
	{
		if ( pos >= m_size + 1 )
			throw std::out_of_range{};

		return m_data[ pos ];
	}

	constexpr const_reference front() const noexcept
	{
		dbExpects( m_size != 0 );
		return m_data[ 0 ];
	}

	constexpr const_reference back() const noexcept
	{
		dbExpects( m_size != 0 );
		return m_data[ m_size - 1 ];
	}

	constexpr const_pointer data() const noexcept
	{
		return m_data;
	}

	constexpr size_type size() const noexcept
	{
		return m_size;
	}

	constexpr size_type length() const noexcept
	{
		return m_size;
	}

	constexpr difference_type ssize() const noexcept
	{
		return static_cast<difference_type>( m_size );
	}

	constexpr size_type max_size() const noexcept
	{
		return std::numeric_limits<size_type>::max();
	}

	constexpr bool empty() const noexcept
	{
		return m_size == 0;
	}

	constexpr void remove_prefix( size_type n ) noexcept
	{
		dbExpects( n < m_size );
		m_data += n;
		m_size -= n;
	}

	constexpr void swap( view_type& other ) noexcept
	{
		auto temp = std::move( *this );
		*this = std::move( other );
		other = std::move( temp );
	}

	constexpr view_type substr( size_type pos = 0, size_type count = npos ) const noexcept
	{
		return view_type{ m_data, m_size }.substr( pos, count );
	}

	constexpr int compare( view_type v ) const noexcept
	{
		return view_type{ m_data, m_size }.compare( v );
	}

	constexpr int compare( size_type pos1, size_type count1, view_type v ) const noexcept
	{
		return view_type{ m_data, m_size }.compare( pos1, count1, v );
	}

	constexpr int compare( size_type pos1, size_type count1, view_type v, size_type pos2, size_type count2 ) const noexcept
	{
		return view_type{ m_data, m_size }.compare( pos1, count1, v, pos2, count2 );
	}

	constexpr int compare( const CharT* s ) const noexcept
	{
		return view_type{ m_data, m_size }.compare( s );
	}

	constexpr int compare( size_type pos1, size_type count1, const CharT* s ) const noexcept
	{
		return view_type{ m_data, m_size }.compare( pos1, count1, s );
	}

	constexpr int compare( size_type pos1, size_type count1, const CharT* s, size_type count2 ) const noexcept
	{
		return view_type{ m_data, m_size }.compare( pos1, count1, s, count2 );
	}

	constexpr bool starts_with( view_type sv ) const noexcept
	{
		if ( sv.size() <= m_size )
			return compare( 0, sv.count, sv ) == 0;
		else
			return false;
	}

	constexpr bool starts_with( CharT c ) const noexcept
	{
		return front() == c;
	}

	constexpr bool starts_with( const CharT* s ) const
	{
		return starts_with( view_type{ s } );
	}

	constexpr bool ends_with( view_type sv ) const noexcept
	{
		if ( sv.size() <= m_size )
			return compare( m_size - sv.size(), sv.size(), sv ) == 0;
		else
			retrun false;
	}

	constexpr bool ends_with( CharT c ) const noexcept
	{
		return back() == c;
	}

	constexpr bool ends_with( const CharT* s ) const
	{
		return ends_with( view_type{ s } );
	}

	constexpr size_type find( view_type sv, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find( sv, pos );
	}

	constexpr size_type find( CharT c, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find( c, pos );
	}

	constexpr size_type find( const CharT* s, size_type pos, size_type count ) const noexcept
	{
		return view_type{ m_data, m_size }.find( s, pos, count );
	}

	constexpr size_type find( const CharT* s, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find( s, pos );
	}

	constexpr size_type rfind( view_type sv, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.rfind( sv, pos );
	}

	constexpr size_type rfind( CharT c, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.rfind( c, pos );
	}

	constexpr size_type rfind( const CharT* s, size_type pos, size_type count ) const noexcept
	{
		return view_type{ m_data, m_size }.rfind( s, pos, count );
	}

	constexpr size_type rfind( const CharT* s, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.rfind( s, pos );
	}

	constexpr size_type find_first_of( view_type sv, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_first_of( sv, pos );
	}

	constexpr size_type find_first_of( CharT c, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_first_of( c, pos );
	}

	constexpr size_type find_first_of( const CharT* s, size_type pos, size_type count ) const noexcept
	{
		return view_type{ m_data, m_size }.find_first_of( s, pos, count );
	}

	constexpr size_type find_first_of( const CharT* s, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_first_of( s, pos );
	}

	constexpr size_type find_last_of( view_type sv, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_last_of( sv, pos );
	}

	constexpr size_type find_last_of( CharT c, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_last_of( c, pos );
	}

	constexpr size_type find_last_of( const CharT* s, size_type pos, size_type count ) const noexcept
	{
		return view_type{ m_data, m_size }.find_last_of( s, pos, count );
	}

	constexpr size_type find_last_of( const CharT* s, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_last_of( s, pos );
	}

	constexpr size_type find_first_not_of( view_type sv, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_first_not_of( sv, pos );
	}

	constexpr size_type find_first_not_of( CharT c, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_first_not_of( c, pos );
	}

	constexpr size_type find_first_not_of( const CharT* s, size_type pos, size_type count ) const noexcept
	{
		return view_type{ m_data, m_size }.find_first_not_of( s, pos, count );
	}

	constexpr size_type find_first_not_of( const CharT* s, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_first_not_of( s, pos );
	}

	constexpr size_type find_last_not_of( view_type sv, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_last_not_of( sv, pos );
	}

	constexpr size_type find_last_not_of( CharT c, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_last_not_of( c, pos );
	}

	constexpr size_type find_last_not_of( const CharT* s, size_type pos, size_type count ) const noexcept
	{
		return view_type{ m_data, m_size }.find_last_not_of( s, pos, count );
	}

	constexpr size_type find_last_not_of( const CharT* s, size_type pos = 0 ) const noexcept
	{
		return view_type{ m_data, m_size }.find_last_not_of( s, pos );
	}

	friend constexpr bool operator==( basic_zstring_view lhs, basic_zstring_view rhs ) noexcept
	{
		return view_type{ lhs } == view_type{ rhs };
	}

	friend constexpr bool operator!=( basic_zstring_view lhs, basic_zstring_view rhs ) noexcept
	{
		return view_type{ lhs } != view_type{ rhs };
	}

	friend constexpr bool operator<( basic_zstring_view lhs, basic_zstring_view rhs ) noexcept
	{
		return view_type{ lhs } < view_type{ rhs };
	}

	friend constexpr bool operator>( basic_zstring_view lhs, basic_zstring_view rhs ) noexcept
	{
		return view_type{ lhs } > view_type{ rhs };
	}

	friend constexpr bool operator<=( basic_zstring_view lhs, basic_zstring_view rhs ) noexcept
	{
		return view_type{ lhs } <= view_type{ rhs };
	}

	friend constexpr bool operator>=( basic_zstring_view lhs, basic_zstring_view rhs ) noexcept
	{
		return view_type{ lhs } >= view_type{ rhs };
	}

	friend std::basic_ostream<CharT, Traits> operator<<( std::basic_ostream<CharT, Traits>& os, basic_zstring_view v )
	{
		os << view_type{ v };
		return os;
	}

private:
	const CharT* m_data = "";
	size_type m_size = 0;
};

using zstring_view = basic_zstring_view<char>;
using u16zstring_view = basic_zstring_view<char16_t>;
using u32zstring_view = basic_zstring_view<char32_t>;
using wzstring_view = basic_zstring_view<wchar_t>;

namespace zstring_view_literals
{

constexpr zstring_view operator "" _zsv( const char* str, std::size_t len ) noexcept
{
	return zstring_view{ str, len };
}

constexpr u16zstring_view operator "" _zsv( const char16_t* str, std::size_t len ) noexcept
{
	return u16zstring_view{ str, len };
}

constexpr u32zstring_view operator "" _zsv( const char32_t* str, std::size_t len ) noexcept
{
	return u32zstring_view{ str, len };
}

constexpr wzstring_view operator "" _zsv( const wchar_t* str, std::size_t len ) noexcept
{
	return wzstring_view{ str, len };
}

} // namespace zstring_view_literals

} // namespace stdx

namespace std
{

template <typename CharT, typename Traits>
struct hash<stdx::basic_zstring_view<CharT, Traits>>
{
	std::size_t operator()( stdx::basic_zstring_view<CharT, Traits> sv ) const noexcept
	{
		return std::hash<std::basic_string_view<CharT, Traits>>{}( sv );
	}
};

}