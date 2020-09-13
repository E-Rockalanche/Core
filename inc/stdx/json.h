#pragma once

#include <stdx/ctype.h>
#include <stdx/simple_map.h>
#include <stdx/type_traits.h>
#include <stdx/zstring_view.h>

#include <charconv>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace stdx
{

namespace detail
{
	template <typename C>
	using key_comp_t = decltype( std::declval<C>().key_comp() );

	template <typename C>
	using key_eq_t = decltype( std::declval<C>().key_eq() );

	template <typename Json>
	class json_iterator
	{
	public:
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using index_type = difference_type;

		// array
		using value_type = typename Json::array_type::value_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;

		// object
		using key_type = typename Json::object_type::key_type;
		using mapped_type = typename Json::object_type::mapped_type;

		constexpr json_iterator() noexcept = default;
		json_iterator( Json& basic_json, index_type index )
			: m_json{ basic_json }
			, m_index{ index }
	#ifdef DEBUG
			, m_type{ basic_json.get_type() }
	#endif
		{}

		json_iterator& operator=( const json_iterator& other )
		{
			m_json = other.m_json;
			m_index = other.m_index;
	#ifdef DEBUG
			dbAssert( m_type == Json::type::null || m_type == other.m_type );
			m_type = other.m_type;
	#endif
		}

		~json_iterator()
		{
	#ifdef DEBUG
			m_json = nullptr;
			m_index = 0;
			m_type = Json::type::null;
	#endif
		}

		operator typename Json::array_type::const_iterator() const { dbExpects( m_type == Json::type::array ); return m_json->arr().begin() + m_index; }
		operator typename Json::object_type::const_iterator() const { dbExpects( m_type == Json::type::object ); return m_json->items().begin() + m_index; }

		json_iterator& operator++() { dbExpects( m_json ); dbExpects( m_type == m_json->get_type() ); dbExpects( m_index < m_json->size() ); ++m_index; }
		json_iterator operator++( int ) { json_iterator copy{ *this }; operator++(); return copy; }

		json_iterator& operator--() { dbExpects( m_json ); dbExpects( m_type == m_json->get_type() ); dbExpects( m_index > 0 ); --m_index; }
		json_iterator operator--( int ) { json_iterator copy{ *this }; operator--(); return copy; }

		json_iterator& operator+=( difference_type n ) { dbExpects( m_json ); dbExpects( m_type == m_json->get_type() ); m_index += n; dbEnsures( 0 <= m_index && m_index <= m_json->size() ); return *this; }
		json_iterator& operator-=( difference_type n ) { dbExpects( m_json ); dbExpects( m_type == m_json->get_type() ); m_index -= n; dbEnsures( 0 <= m_index && m_index <= m_json->size() ); return *this; }

		// array
		value_type& operator*() const { dbExpects( m_json ); dbExpects( m_type == m_json->get_type() ); return m_json->arr()[ m_index ]; }
		value_type* operator->() const { dbExpects( m_json ); dbExpects( m_type == m_json->get_type() ); return &m_json->arr()[ m_index ]; }
		value_type& operator[]( index_type index ) const { dbExpects( m_json ); dbExpects( m_type == m_json->get_type() ); return m_json->arr()[ m_index + index ]; }

		// object
		const key_type& key() const { dbExpects( m_json ); dbExpects( m_type == m_json->get_type() ); return ( m_json->items().begin() + m_index )->first; }
		const mapped_type& value() const { dbExpects( m_json ); dbExpects( m_type == m_json->get_type() ); return ( m_json->items().begin() + m_index )->second; }

		friend json_iterator operator+( json_iterator it, difference_type n ) { return it += n; }
		friend json_iterator operator+( difference_type n, json_iterator it ) { return it += n; }
		friend json_iterator operator-( json_iterator it, difference_type n ) { return it -= n; }
		friend json_iterator operator-( json_iterator lhs, json_iterator rhs ) { dbExpects( lhs.m_json == rhs.m_json ); dbExpects( lhs.m_type == lhs.m_json->get_type() ); dbExpects( rhs.m_type == rhs.m_json->get_type() ); return lhs.m_index - rhs.m_index; }

		friend bool operator==( json_iterator lhs, json_iterator rhs ) { dbExpects( lhs.m_json == rhs.m_json ); dbExpects( lhs.m_type == lhs.m_json->get_type() ); dbExpects( rhs.m_type == rhs.m_json->get_type() ); return lhs.m_index == rhs.m_index; }
		friend bool operator!=( json_iterator lhs, json_iterator rhs ) { dbExpects( lhs.m_json == rhs.m_json ); dbExpects( lhs.m_type == lhs.m_json->get_type() ); dbExpects( rhs.m_type == rhs.m_json->get_type() ); return lhs.m_index != rhs.m_index; }
		friend bool operator<( json_iterator lhs, json_iterator rhs ) { dbExpects( lhs.m_json == rhs.m_json ); dbExpects( lhs.m_type == lhs.m_json->get_type() ); dbExpects( rhs.m_type == rhs.m_json->get_type() ); return lhs.m_index < rhs.m_index; }
		friend bool operator>( json_iterator lhs, json_iterator rhs ) { dbExpects( lhs.m_json == rhs.m_json ); dbExpects( lhs.m_type == lhs.m_json->get_type() ); dbExpects( rhs.m_type == rhs.m_json->get_type() ); return lhs.m_index > rhs.m_index; }
		friend bool operator<=( json_iterator lhs, json_iterator rhs ) { dbExpects( lhs.m_json == rhs.m_json ); dbExpects( lhs.m_type == lhs.m_json->get_type() ); dbExpects( rhs.m_type == rhs.m_json->get_type() ); return lhs.m_index <= rhs.m_index; }
		friend bool operator>=( json_iterator lhs, json_iterator rhs ) { dbExpects( lhs.m_json == rhs.m_json ); dbExpects( lhs.m_type == lhs.m_json->get_type() ); dbExpects( rhs.m_type == rhs.m_json->get_type() ); return lhs.m_index >= rhs.m_index; }

	private:
		Json* m_json = nullptr;
		index_type m_index = 0;

		friend class Json;

	#ifdef DEBUG
		Json::type m_type = Json::type::null;
	#endif
	};

} // namespace detail

class json_exception : public std::exception
{
public:
	using std::exception::exception;
};

template <typename String, template<class> typename Vector, template<class, class> typename Map>
class basic_json
{
public:
	struct null_type {};

	using number_type = double;

	using bool_type = bool;

	using string_type = String;
	using char_type = std::remove_const_t<typename stdx::container_traits<String>::value_type>;
	using view_type = std::basic_string_view<char_type>;
	using zview_type = stdx::basic_zstring_view<char_type>;

	using array_type = Vector<basic_json>;

	using key_type = string_type;
	using mapped_type = basic_json;
	using object_type = Map<key_type, mapped_type>;

	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using iterator = detail::json_iterator<basic_json>;
	using const_iterator = detail::json_iterator<const basic_json>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static constexpr size_type npos = view_type::npos;

	enum class type
	{
		null,
		number,
		boolean,
		string,
		array,
		object
	};

	// construction

	basic_json() noexcept = default;
	basic_json( const basic_json& other ) = default;
	basic_json( basic_json&& other ) noexcept : m_data{ std::exchange( other.m_data, null_type{} ) } {}

	basic_json( std::nullptr_t ) noexcept {}
	basic_json( number_type value ) noexcept : m_data{ value } {}
	basic_json( bool_type value ) noexcept : m_data{ value } {}

	basic_json( const string_type& value ) noexcept : m_data{ value } {}
	basic_json( string_type&& value ) noexcept : m_data{ std::move( value ) } {}
	basic_json( view_type value ) : basic_json( string_type{ value } ) {}
	basic_json( const char_type* value ) : basic_json( string_type{ value } ) {}
	template <std::size_t N>
	basic_json( const char_type( &value )[ N ] ) : basic_json( string_type{ value } ) {}

	basic_json( const array_type& value ) : m_data{ value } {}
	basic_json( array_type&& value ) : m_data{ std::move( value ) } {}
	basic_json( std::initializer_list<basic_json> init ) : basic_json( array_type{ init } ) {}

	basic_json( const object_type& value ) : m_data{ value } {}
	basic_json( object_type&& value ) : m_data{ std::move( value ) } {}
	basic_json( std::initializer_list<std::pair<std::string, basic_json>> init ) : basic_json( object_type{ init } ) {}

	template <typename T>
	basic_json( T&& t );

	static basic_json null() { return basic_json{}; }
	static basic_json boolean( bool_type value = false ) { return basic_json{ value }; }
	static basic_json number( number_type value = 0 ) { return basic_json{ value }; }

	static basic_json string() { return basic_json{ string_type{} }; }
	static basic_json string( const string_type& value ) { return basic_json{ value }; }
	static basic_json string( string_type&& value ) { return basic_json{ std::move( value ) }; }
	static basic_json string( view_type value ) { return basic_json{ value }; }
	static basic_json string( const char_type* value ) { return basic_json{ value }; }
	template <std::size_t N>
	static basic_json string( const char_type( &value )[ N ] ) { return basic_json{ value }; }

	static basic_json array() { return basic_json{ array_type{} }; }
	static basic_json array( const array_type& value ) { return basic_json{ value }; }
	static basic_json array( array_type&& value ) { return basic_json{ std::move( value ) }; }
	static basic_json array( std::initializer_list<basic_json> init ) { return basic_json{ init }; }

	static basic_json object() { return basic_json{ object_type{} }; }
	static basic_json object( const object_type& value ) { return basic_json{ value }; }
	static basic_json object( object_type&& value ) { return basic_json{ std::move( value ) }; }
	static basic_json object( std::initializer_list<object_type::value_type> init ) { return basic_json{ init }; }

	// assignment

	basic_json& operator=( const basic_json& other ) = default;
	basic_json& operator=( basic_json&& other ) noexcept = default;

	// conversion

	template <typename T>
	T get() const;
	template <>
	bool_type get<bool_type>() const { return std::get<bool_type>( m_data ); }
	template <>
	number_type get<number_type>() const { return std::get<number_type>( m_data ); }
	template <>
	string_type get<string_type>() const { return std::get<string_type>( m_data ); }
	template <>
	array_type get<array_type>() const { return std::get<array_type>( m_data ); }
	template <>
	object_type get<object_type>() const { return std::get<object_type>( m_data ); }

	template <typename T>
	T get() &&;
	template <>
	bool_type get<bool_type>() && { return std::get<bool_type>( m_data ); }
	template <>
	number_type get<number_type>() && { return std::get<number_type>( m_data ); }
	template <>
	string_type get<string_type>() && { return std::move( std::get<string_type>( m_data ) ); }
	template <>
	array_type get<array_type>() && { return std::move( std::get<array_type>( m_data ) ); }
	template <>
	object_type get<object_type>() && { return std::move( std::get<object_type>( m_data ) ); }

	template <typename T>
	operator T() const { return get<T>(); }

	template <typename T>
	operator T() && { return get<T>(); }

	// serialization

	std::basic_string<char_type> dump( size_type tab_width = 0 ) const
	{
		std::basic_string<char_type> s;
		dump_imp( s, tab_width, 0 );
		return s;
	}

	static basic_json parse( zview_type v )
	{
		size_type i = 0;
		while ( stdx::isspace( v[ i ] ) )
			++i;
		return parse_imp( v, i );
	}

	// iterators

	iterator begin() { dbExpects( is_array() || is_object() ); return iterator( *this, 0 ); }
	iterator end() { dbExpects( is_array() || is_object() ); return iterator( *this, size() ); }
	const_iterator begin() const { dbExpects( is_array() || is_object() ); return const_iterator( *this, 0 ); }
	const_iterator end() const { dbExpects( is_array() || is_object() ); return const_iterator( *this, size() ); }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }
	reverse_iterator rbegin() { return reverse_iterator( end() ); }
	reverse_iterator rend() { return reverse_iterator( begin() ); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator( end() ); }
	const_reverse_iterator rend() const { return const_reverse_iterator( begin() ); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator( end() ); }
	const_reverse_iterator crend() const { return const_reverse_iterator( begin() ); }

	// query

	type get_type() const noexcept { return static_cast<type>( m_data.index() ); }

	bool is_null() const noexcept { return get_type() == type::null; }
	bool is_bool() const noexcept { return get_type() == type::boolean; }
	bool is_number() const noexcept { return get_type() == type::number; }
	bool is_string() const noexcept { return get_type() == type::string; }
	bool is_array() const noexcept { return get_type() == type::array; }
	bool is_object() const noexcept { return get_type() == type::object; }

	bool empty() const
	{
		switch ( get_type() )
		{
			case type::string: return str().empty();
			case type::array: return arr().empty();
			case type::object: return items().empty();

			default: throw json_exception( "current type has no empty()" );
		}
	}

	size_type size() const
	{
		switch ( get_type() )
		{
			case type::string: return str().size();
			case type::array: return arr().size();
			case type::object: return items().size();

			default: throw json_exception( "current type has no size()" );
		}
	}

	size_type max_size() const
	{
		switch ( get_type() )
		{
			case type::string: return str().max_size();
			case type::array: return arr().max_size();
			case type::object: return items().max_size();

			default: throw json_exception( "current type has no max_size()" );
		}
	}

	// capacity

	void reserve( size_type capacity )
	{
		switch ( get_type() )
		{
			case type::string: return str().reserve( capacity );
			case type::array: return arr().reserve( capacity );
			case type::object: return items().reserve( capacity );

			default: throw json_exception( "current type has no reserve(size_type)" );
		}
	}

	size_type capacity() const
	{
		switch ( get_type() )
		{
			case type::string: return str().capacity();
			case type::array: return arr().capacity();
			case type::object: return items().capacity();

			default: throw json_exception( "current type has no capacity()" );
		}
	}

	void shrink_to_fit()
	{
		switch ( get_type() )
		{
			case type::string: return str().shrink_to_fit();
			case type::array: return arr().shrink_to_fit();
			case type::object: return items().shrink_to_fit();

			default: throw json_exception( "current type has no shrink_to_fit()" );
		}
	}

	// modifiers

	void clear()
	{
		switch ( get_type() )
		{
			case type::string: str().clear(); break;
			case type::array: arr().clear(); break;
			case type::object: items().clear(); break;

			default: throw json_exception( "current type has no clear()" );
		}
	}

	void resize( size_type count )
	{
		switch ( get_type() )
		{
			case type::string: str().resize( count ); break;
			case type::array: arr().resize( count ); break;

			default: throw json_exception( "current type has no resize(size_type)" );
		}
	}

	template <typename T>
	void resize( size_type count, const T& value )
	{
		switch ( get_type() )
		{
			case type::string: str().resize( count, value ); break;
			case type::array: arr().resize( count, value ); break;

			default: throw json_exception( "current type has no resize(size_type, const T&)" );
		}
	}

	void swap( basic_json&& other )
	{
		auto temp = std::move( m_data );
		m_data = std::move( other.m_data );
		other.m_data = std::move( temp );
	}

	// string operators

	string_type& str() { return std::get<string_type>( m_data ); }
	const string_type& str() const { return std::get<string_type>( m_data ); }

	basic_json& assign( size_type count, char_type c ) { return *this = string_type( count, c ); }
	basic_json& assign( const string_type& s ) { return *this = s; }
	basic_json& assign( const string_type& s, size_type pos, size_type count ) { return *this = string_type( s, pos, count ); }
	basic_json& assign( string_type&& s ) noexcept { return *this = std::move( s ); }
	basic_json& assign( const char_type* s, size_type count ) { return *this = string_type( s, count ); }
	basic_json& assign( const char_type* s ) { return *this = string_type( s ); }
	template <typename InputIt>
	basic_json& assign( InputIt first, InputIt last ) { return *this = string_type( first, last ); }
	basic_json& assign( std::initializer_list<char_type> init ) { return *this = string_type( init ); }
	template <typename T>
	basic_json& assign( const T& t ) { return *this = string_type( t ); }
	template <typename T>
	basic_json& assign( const T& t, size_type pos, size_type count = npos ) { return *this = string_type( t, pos, count ); }
	basic_json& assign( const basic_json& j ) { return *this = std::get<string_type>( j.m_data ); }
	basic_json& assign( const basic_json& j, size_type pos, size_type count ) { return *this = string_type( std::get<string_type>( j.m_data ), pos, count ); }
	basic_json& assign( basic_json&& j ) noexcept
	{
		m_data = std::move( std::get<string_type>( j.m_data ) );
		j = nullptr;
		return *this;
	}

	const char* c_str() const { return str().c_str(); }
	operator view_type() const { return str(); }
	operator zview_type() const { auto& s = str(); return zview_type{ s.c_str(), s.size() }; }
	size_type length() const { return str().length(); }

	basic_json& append( size_type count, char_type c ) { str().append( count, c ); return *this; }
	basic_json& append( const string_type& s ) { str().append( s ); return *this; }
	basic_json& append( const string_type& s, size_type pos, size_type count = npos ) { str().append( s, pos, count ); return *this; }
	basic_json& append( const char_type* s, size_type count ) { str().append( s, count ); return *this; }
	basic_json& append( const char_type* s ) { str().append( s ); return *this; }
	template <typename InputIt>
	basic_json& append( InputIt first, InputIt last ) { str().append( first, last ); return *this; }
	basic_json& append( std::initializer_list<char_type> init ) { str().append( init ); return *this; }
	template <typename T>
	basic_json& append( const T& t ) { str().append( t ); return *this; }
	template <typename T>
	basic_json& append( const T& t, size_type pos, size_type count = npos ) { str().append( t, pos, count ); return *this; }
	basic_json& append( const basic_json& j ) { str().append( std::get<string_type>( j.m_data ) ); return *this; }
	basic_json& append( const basic_json& j, size_type pos, size_type count = npos ) { str().append( std::get<string_type>( j.m_data ), pos, count ); return *this; }

	basic_json& operator+=( const string_type& s ) { str() += s; return *this; }
	basic_json& operator+=( const char_type c ) { str() += c; return *this; }
	basic_json& operator+=( const char_type* s ) { str() += s; return *this; }
	basic_json& operator+=( std::initializer_list<char_type> init ) { str() += init; return *this; }
	template <typename T>
	basic_json& operator+=( const T& t ) { str() += t; return *this; }
	basic_json& operator+=( const basic_json& j ) { str() += std::get<string_type>( j.m_data ); return *this; }

	int compare( const string_type& s ) const { return str().compare( s ); }
	int compare( size_type pos1, size_type count1, const string_type& s ) const { return str().compare( pos1, count1, s ); }
	int compare( size_type pos1, size_type count1, const string_type& s, size_type pos2, size_type count2 = npos ) const { return str().compare( pos1, count1, s, pos2, count2 ); }
	int compare( const char_type* s ) const { return str().compare( s ); }
	int compare( size_type pos1, size_type count1, const char_type* s ) const { return str().compare( pos1, count1, s ); }
	int compare( size_type pos1, size_type count1, const char_type* s, size_type count2 ) const { return str().compare( pos1, count1, s, count2 ); }
	template <typename T>
	int compare( const T& s ) const { return str().compare( s ); }
	template <typename T>
	int compare( size_type pos1, size_type count1, const T& s ) const { return str().compare( pos1, count1, s ); }
	template <typename T>
	int compare( size_type pos1, size_type count1, const T& s, size_type pos2, size_type count2 = npos ) const { return str().compare( pos1, count1, s, pos2, count2 ); }
	int compare( const basic_json& j ) const { return str().compare( std::get<string_type>( j.m_data ) ); }
	int compare( size_type pos1, size_type count1, const basic_json& j ) const { return str().compare( pos1, count1, std::get<string_type>( j.m_data ) ); }
	int compare( size_type pos1, size_type count1, const basic_json& j, size_type pos2, size_type count2 = npos ) const { return str().compare( pos1, count1, std::get<string_type>( j.m_data ), pos2, count2 ); }

	bool starts_with( view_type sv ) const { return view_type{ str() }.substr( 0, sv.size() ) == sv; }
	bool starts_with( const char_type* s ) const { return starts_with( view_type{ s } ); }
	bool starts_with( char_type c ) const
	{
		auto& s = str();
		return s.empty() ? false : ( s.front() == c );
	}

	bool ends_with( view_type sv ) const
	{
		auto& s = str();
		return view_type{ s }.substr( s.size() - ( std::min )( sv.size(), s.size() ) ) == sv;
	}
	bool ends_with( const char_type* s ) const { return ends_with( view_type{ s } ); }
	bool ends_with( char_type c ) const
	{
		auto& s = str();
		return s.empty() ? false : ( s.back() == c );
	}

	basic_json& replace( size_type pos, size_type count, const string_type& s ) { str().replace( pos, count, s ); return *this; }
	basic_json& replace( string_type::const_iterator first, string_type::const_iterator last, const string_type& s ) { str().replace( first, last, s ); return *this; }
	basic_json& replace( size_type pos, size_type count, const string_type& s, size_type pos2, size_type count2 = npos ) { str().replace( pos, count, s, pos2, count2 ); return *this; }
	template <typename InputIt>
	basic_json& replace( string_type::const_iterator first, string_type::const_iterator last, InputIt first2, InputIt last2 ) { str().replace( first, last, first2, last2 ); return *this; }
	basic_json& replace( size_type pos, size_type count, const char_type* s, size_type count2 ) { str().replace( pos, count, s, count2 ); return *this; }
	basic_json& replace( string_type::const_iterator first, string_type::const_iterator last, const char_type* s, size_type count ) { str().replace( first, last, s, count ); return *this; }
	basic_json& replace( size_type pos, size_type count, const char_type* s ) { str().replace( pos, count, s ); return *this; }
	basic_json& replace( string_type::const_iterator first, string_type::const_iterator last, const char_type* s ) { str().replace( first, last, s ); return *this; }
	basic_json& replace( size_type pos, size_type count, size_type count2, char_type c ) { str().replace( pos, count, count2, c ); return *this; }
	basic_json& replace( string_type::const_iterator first, string_type::const_iterator last, size_type count2, char_type c ) { str().replace( first, last, count2, c ); return *this; }
	basic_json& replace( string_type::const_iterator first, string_type::const_iterator last, std::initializer_list<char_type> init ) { str().replace( first, last, init ); return *this; }
	template <typename T>
	basic_json& replace( size_type pos, size_type count, const T& t ) { str().replace( pos, count, t ); return *this; }
	template <typename T>
	basic_json& replace( string_type::const_iterator first, string_type::const_iterator last, const T& t ) { str().replace( first, last, t ); return *this; }
	template <typename T>
	basic_json& replace( size_type pos, size_type count, const T& t, size_type pos2, size_type count2 = npos ) { str().replace( pos, count, t, pos2, count2 ); return *this; }
	basic_json& replace( size_type pos, size_type count, const basic_json& j ) { str().replace( pos, count, std::get<string_type>( j.m_data ) ); return *this; }
	basic_json& replace( string_type::const_iterator first, string_type::const_iterator last, const basic_json& j ) { str().replace( first, last, std::get<string_type>( j.m_data ) ); return *this; }
	basic_json& replace( size_type pos, size_type count, const basic_json& j, size_type pos2, size_type count2 = npos ) { str().replace( pos, count, std::get<string_type>( j.m_data ), pos2, count2 ); return *this; }

	view_type substr( size_type pos = 0, size_type count = npos ) const { return str().substr( pos, count ); }

	size_type copy( char_type* dest, size_type count, size_type pos = 0 ) const { return str().copy( dest, count, pos ); }

	// no default pos to avoid conflit with map::find
	size_type find( const string_type& s, size_type pos ) const { return str().find( s, pos ); }
	size_type find( const char_type* s, size_type pos, size_type count ) const { return str().find( s, pos, count ); }
	size_type find( const char_type* s, size_type pos ) const { return str().find( s, pos ); }
	size_type find( char_type c, size_type pos ) const { return str().find( c, pos ); }
	template <typename T>
	size_type find( const T& t, size_type pos ) const { return str().find( t, pos ); }
	size_type find( const basic_json& j, size_type pos ) const { return str().find( std::get<string_type>( j.m_data ), pos ); }

	size_type rfind( const string_type& s, size_type pos ) const { return str().rfind( s, pos ); }
	size_type rfind( const char_type* s, size_type pos, size_type count ) const { return str().rfind( s, pos, count ); }
	size_type rfind( const char_type* s, size_type pos ) const { return str().rfind( s, pos ); }
	size_type rfind( char_type c, size_type pos ) const { return str().rfind( c, pos ); }
	template <typename T>
	size_type rfind( const T& t, size_type pos ) const { return str().rfind( t, pos ); }
	size_type rfind( const basic_json& j, size_type pos ) const { return str().rfind( std::get<string_type>( j.m_data ), pos ); }

	size_type find_first_of( const string_type& s, size_type pos ) const { return str().find_first_of( s, pos ); }
	size_type find_first_of( const char_type* s, size_type pos, size_type count ) const { return str().find_first_of( s, pos, count ); }
	size_type find_first_of( const char_type* s, size_type pos ) const { return str().find_first_of( s, pos ); }
	size_type find_first_of( char_type c, size_type pos ) const { return str().find_first_of( c, pos ); }
	template <typename T>
	size_type find_first_of( const T& t, size_type pos ) const { return str().find_first_of( t, pos ); }
	size_type find_first_of( const basic_json& j, size_type pos ) const { return str().find_first_of( std::get<string_type>( j.m_data ), pos ); }

	size_type find_first_not_of( const string_type& s, size_type pos ) const { return str().find_first_not_of( s, pos ); }
	size_type find_first_not_of( const char_type* s, size_type pos, size_type count ) const { return str().find_first_not_of( s, pos, count ); }
	size_type find_first_not_of( const char_type* s, size_type pos ) const { return str().find_first_not_of( s, pos ); }
	size_type find_first_not_of( char_type c, size_type pos ) const { return str().find_first_not_of( c, pos ); }
	template <typename T>
	size_type find_first_not_of( const T& t, size_type pos ) const { return str().find_first_not_of( t, pos ); }
	size_type find_first_not_of( const basic_json& j, size_type pos ) const { return str().find_first_not_of( std::get<string_type>( j.m_data ), pos ); }

	size_type find_last_of( const string_type& s, size_type pos ) const { return str().find_last_of( s, pos ); }
	size_type find_last_of( const char_type* s, size_type pos, size_type count ) const { return str().find_last_of( s, pos, count ); }
	size_type find_last_of( const char_type* s, size_type pos ) const { return str().find_last_of( s, pos ); }
	size_type find_last_of( char_type c, size_type pos ) const { return str().find_last_of( c, pos ); }
	template <typename T>
	size_type find_last_of( const T& t, size_type pos ) const { return str().find_last_of( t, pos ); }
	size_type find_last_of( const basic_json& j, size_type pos ) const { return str().find_last_of( std::get<string_type>( j.m_data ), pos ); }

	size_type find_last_not_of( const string_type& s, size_type pos ) const { return str().find_last_not_of( s, pos ); }
	size_type find_last_not_of( const char_type* s, size_type pos, size_type count ) const { return str().find_last_not_of( s, pos, count ); }
	size_type find_last_not_of( const char_type* s, size_type pos ) const { return str().find_last_not_of( s, pos ); }
	size_type find_last_not_of( char_type c, size_type pos ) const { return str().find_last_not_of( c, pos ); }
	template <typename T>
	size_type find_last_not_of( const T& t, size_type pos ) const { return str().find_last_not_of( t, pos ); }
	size_type find_last_not_of( const basic_json& j, size_type pos ) const { return str().find_last_not_of( std::get<string_type>( j.m_data ), pos ); }

	// array operators

	array_type& arr() { return std::get<array_type>( m_data ); }
	const array_type& arr() const { return std::get<array_type>( m_data ); }

	basic_json& at( size_type index ) noexcept { return arr().at( index ); }
	const basic_json& at( size_type index ) const noexcept { return arr().at( index ); }

	basic_json& operator[]( size_type index ) noexcept { return arr()[ index ]; }
	const basic_json& operator[]( size_type index ) const noexcept { return arr()[ index ]; }

	basic_json& front() { return arr().front(); }
	const basic_json& front() const { return arr().front(); }

	basic_json& back() { return arr().back(); }
	const basic_json& back() const { return arr().back(); }

	basic_json* data() { return arr().data(); }
	const basic_json* data() const { return arr().data(); }

	void insert( array_type::const_iterator pos, const basic_json& j ) { arr().insert( pos, j ); }
	void insert( array_type::const_iterator pos, basic_json&& j ) { arr().insert( pos, std::move( j ) ); }
	void insert( array_type::const_iterator pos, size_type count, const basic_json& j ) { arr().insert( pos, count, j ); }
	template <typename InputIt>
	void insert( array_type::const_iterator pos, InputIt first, InputIt last ) { arr().insert( pos, first, last ); }
	void insert( array_type::const_iterator pos, std::initializer_list<basic_json> init ) { arr().insert( pos, init ); }

	template <typename... Args>
	void emplace( array_type::const_iterator pos, Args... args ) { arr().emplace( pos, std::forward<Args>( args )... ); }

	array_type::iterator erase( array_type::const_iterator pos ) { return arr().erase( pos ); }
	array_type::iterator erase( array_type::const_iterator first, array_type::const_iterator last ) { return arr().erase( first, last ); }

	void push_back( const basic_json& j ) { return arr().push_back( j ); }
	void push_back( basic_json&& j ) { return arr().push_back( std::move( j ) ); }

	template <typename... Args>
	void emplace_back( Args&&... args ) { return arr().emplace_back( std::forward<Args>( args )... ); }

	void pop_back() { arr().pop_back(); }

	// object operators

	object_type& items() { return std::get<object_type>( m_data ); }
	const object_type& items() const { return std::get<object_type>( m_data ); }

	basic_json& operator[]( const string_type& key ) noexcept
	{
		if ( is_null() )
			m_data = object_type{};
		return items()[ key ];
	}

	basic_json& operator[]( string_type&& key ) noexcept
	{
		if ( is_null() )
			m_data = object_type{};
		return items()[ std::move( key ) ];
	}

	std::pair<object_type::iterator, bool> insert( const object_type::value_type& pair ) { return items().insert( pair ); }
	std::pair<object_type::iterator, bool> insert( object_type::value_type&& pair ) { return items().insert( std::move( pair ) ); }

	template <typename M>
	std::pair<object_type::iterator, bool> insert_or_assign( const key_type& k, M&& obj ) { return items().insert_or_assign( k, std::forward<M>( obj ) ); }
	template <typename M>
	std::pair<object_type::iterator, bool> insert_or_assign( key_type&& k, M&& obj ) { return items().insert_or_assign( std::move( k ), std::forward<M>( obj ) ); }

	template <typename... Args>
	std::pair<object_type::iterator, bool> emplace( Args&&... args ) { return items().emplace( std::forward<Args>( args )... ); }

	template <typename... Args>
	std::pair<object_type::iterator, bool> try_emplace( const key_type& k, Args&&... args ) { return items().try_emplace( k, std::forward<Args>( args )... ); }
	template <typename... Args>
	std::pair<object_type::iterator, bool> try_emplace( key_type&& k, Args&&... args ) { return items().try_emplace( std::move( k ), std::forward<Args>( args )... ); }

	object_type::iterator erase( object_type::iterator pos ) { return items().erase( pos ); }
	object_type::iterator erase( object_type::const_iterator pos ) { return items().erase( pos ); }
	object_type::iterator erase( object_type::const_iterator first, object_type::const_iterator last ) { return items().erase( first, last ); }
	size_type erase( const key_type k ) { return items().erase( k ); }

	object_type::iterator find( const key_type& k ) { return items().find( k ); }
	object_type::const_iterator find( const key_type& k ) const { return items().find( k ); }
	template <typename K>
	object_type::iterator find( const K& k ) { return items().find( k ); }
	template <typename K>
	object_type::const_iterator find( const K& k ) const { return items().find( k ); }

	bool contains( const key_type& k ) const
	{
		auto& object = items();
		return object.find( k ) != object.end();
	}
	template <typename K>
	bool contains( const K& k ) const
	{
		auto& object = items();
		return object.find( k ) != object.end();
	}

	template <std::enable_if_t<stdx::is_detected_v<detail::key_comp_t, object_type>, int> = 0>
	auto key_comp() const { return items().key_comp(); }

	template <std::enable_if_t<stdx::is_detected_v<detail::key_eq_t, object_type>, int> = 0>
	auto key_eq() const { return items().key_eq(); }

	// non member functions

	friend bool operator==( const basic_json& lhs, const basic_json& rhs ) noexcept;
	friend bool operator!=( const basic_json& lhs, const basic_json& rhs ) noexcept;
	friend bool operator<( const basic_json& lhs, const basic_json& rhs ) noexcept;
	friend bool operator>( const basic_json& lhs, const basic_json& rhs ) noexcept;
	friend bool operator<=( const basic_json& lhs, const basic_json& rhs ) noexcept;
	friend bool operator>=( const basic_json& lhs, const basic_json& rhs ) noexcept;

private:
	void dump_imp( std::basic_string<char_type>& s, size_type tab_width, size_type depth ) const;

	static void serialize_to( std::basic_string<char_type>& s, view_type v );

	static basic_json parse_imp( zview_type v, size_type& pos );

	static string_type parse_string( zview_type v, size_type& pos );
	static number_type parse_number( zview_type v, size_type& pos );
	static array_type parse_array( zview_type v, size_type& pos );
	static object_type parse_object( zview_type v, size_type& pos );

private:

	// order must match enum type order
	std::variant<null_type, double, bool, string_type, array_type, object_type> m_data;
};


template <typename String, template<class> typename Vector, template<class, class> typename Map>
template <typename T>
basic_json<String, Vector, Map>::basic_json( T&& t )
{
	if constexpr ( std::is_integral_v<T> || std::is_floating_point_v<T> )
	{
		m_data = static_cast<number_type>( t );
	}
	else if constexpr ( std::is_convertible_v<T, view_type> )
	{
		m_data = string_type( view_type( t ) );
	}
	else if ( stdx::is_list_v<T> || stdx::is_array_like_v<T> || stdx::is_set_v<T> )
	{
		m_data = array_type{ std::begin( t ), std::end( t ) };
	}
	else if ( stdx::is_map_v<T> )
	{
		m_data = object_type{ std::begin( t ), std::end( t ) };
	}
	else
	{
		static_assert( "cannot convert T to basic_json" );
	}
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
template <typename T>
T basic_json<String, Vector, Map>::get() const
{
	if constexpr ( std::is_integral_v<T> || std::is_floating_point_v<T> )
	{
		return static_cast<T>( std::get<number_type>( m_data ) );
	}
	else if constexpr ( std::is_convertible_v<T, view_type> )
	{
		return view_type{ str() };
	}
	else if constexpr ( stdx::is_list_v<T> || stdx::is_set_v<T> )
	{
		auto& array = arr();
		return T{ array.begin(), array.end() };
	}
	else if constexpr ( stdx::is_map_v<T> )
	{
		auto& object = items();
		return T{ object.begin(), object.end() };
	}
	else
	{
		static_assert( false, "cannot convert basic_json to T" );
		return T{};
	}
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
template <typename T>
T basic_json<String, Vector, Map>::get() &&
{
	if constexpr ( std::is_integral_v<T> || std::is_floating_point_v<T> )
	{
		return static_cast<T>( std::get<number_type>( m_data ) );
	}
	else if constexpr ( std::is_convertible_v<T, view_type> )
	{
		return view_type{ str() };
	}
	else if constexpr ( stdx::is_list_v<T> )
	{
		T list;
		for ( auto& element : arr() )
			list.emplace_back( std::move( element ) );
		return list;
	}
	else if constexpr ( stdx::is_set_v<T> )
	{
		T set;
		for ( auto& element : arr() )
			set.emplace( std::move( element ) );
		return set;
	}
	else if constexpr ( stdx::is_map_v<T> )
	{
		T map;
		for ( auto& pair : items() )
			map.emplace( std::move( pair.first ), std::move( pair.second ) );
		return map;
	}
	else
	{
		static_assert( false, "cannot determine conversion for T" );
	}
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
 bool operator==( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs )
{
	const auto type = lhs.get_type();
	if ( type != rhs.get_type() )
		throw json_exception( "basic_json type compare mismatch" );

	switch ( type )
	{
		case basic_json::type::null: return true;
		case basic_json::type::boolean: return std::get<basic_json::bool_type>( lhs.m_data ) == std::get<basic_json::bool_type>( rhs.m_data );
		case basic_json::type::number: return std::get<basic_json::number_type>( lhs.m_data ) == std::get<basic_json::number_type>( rhs.m_data );
		case basic_json::type::string: return std::get<basic_json::string_type>( lhs.m_data ) == std::get<basic_json::string_type>( rhs.m_data );
		case basic_json::type::array: return std::get<basic_json::array_type>( lhs.m_data ) == std::get<basic_json::array_type>( rhs.m_data );
		case basic_json::type::object: return std::get<basic_json::object_type>( lhs.m_data ) == std::get<basic_json::object_type>( rhs.m_data );
	}
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
 bool operator!=( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs ) { return !( lhs == rhs ); }

template <typename String, template<class> typename Vector, template<class, class> typename Map>
 bool operator<( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs )
{
	const auto type = lhs.get_type();
	if ( type != rhs.get_type() )
		throw json_exception( "basic_json type compare mismatch" );

	switch ( type )
	{
		case basic_json::type::null: return false;
		case basic_json::type::boolean: return std::get<basic_json::bool_type>( lhs.m_data ) < std::get<basic_json::bool_type>( rhs.m_data );
		case basic_json::type::number: return std::get<basic_json::number_type>( lhs.m_data ) < std::get<basic_json::number_type>( rhs.m_data );
		case basic_json::type::string: return std::get<basic_json::string_type>( lhs.m_data ) < std::get<basic_json::string_type>( rhs.m_data );
		case basic_json::type::array: return std::get<basic_json::array_type>( lhs.m_data ) < std::get<basic_json::array_type>( rhs.m_data );
		case basic_json::type::object: return std::get<basic_json::object_type>( lhs.m_data ) < std::get<basic_json::object_type>( rhs.m_data );
	}
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
 bool operator>( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs ) { return rhs < lhs; }

template <typename String, template<class> typename Vector, template<class, class> typename Map>
 bool operator<=( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs ) { return !( lhs > rhs ); }

template <typename String, template<class> typename Vector, template<class, class> typename Map>
 bool operator>=( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs ) { return !( lhs < rhs ); }

template <typename String, template<class> typename Vector, template<class, class> typename Map>
void basic_json<String, Vector, Map>::dump_imp( std::basic_string<char_type>& s, size_type tab_width, size_type depth ) const
{
	switch ( get_type() )
	{
		case type::null: s += "null"; break;

		case type::number: s += std::to_string( get<number_type>() ); break;

		case type::boolean: s += ( get<bool>() ? "true" : "false" ); break;

		case type::string: serialize_to( s, str() ); break;

		case type::array:
		{
			s += ( tab_width ) ? "[\n" : "[";
			depth += tab_width;
			auto& array = arr();
			for ( auto first = array.begin(), last = array.end(), it = first; it != last; ++it )
			{
				if ( it != first )
					s += ( tab_width ) ? ",\n" : ",";
				s.append( depth, ' ' );
				it->dump_imp( s, tab_width, depth );
			}
			depth -= tab_width;
			s.append( depth, ' ' );
			s += ']';
			break;
		}

		case type::object:
		{
			s.append( depth, ' ' );
			s += ( tab_width ) ? "{\n" : "{";
			depth += tab_width;
			auto& map = items();
			for ( auto first = map.begin(), last = map.end(), it = first; it != last; ++it )
			{
				if ( it != first )
					s += ( tab_width ) ? ",\n" : ",";
				s.append( depth * tab_width, ' ' );
				serialize_to( s, it->first );
				s += ( tab_width ) ? ": " : ":";
				it->second.dump_imp( s, tab_width, depth );
			}
			depth -= tab_width;
			s.append( depth, ' ' );
			s += '}';
			break;
		}
	}
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
void basic_json<String, Vector, Map>::serialize_to( std::basic_string<char_type>& s, view_type v )
{
	s.reserve( s.size() + v.size() );
	for ( auto c : v )
	{
		switch ( c )
		{
			case '\b':s += "\\b"; break;
			case '\f':s += "\\f"; break;
			case '\n': s += "\\n"; break;
			case '\r': s += "\\r"; break;
			case '\t': s += "\\t"; break;
			case '"': s += "\\\""; break;
			case '\\': s += "\\\\"; break;
			default: s += c; break;
		}
	}
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
basic_json<String, Vector, Map> basic_json<String, Vector, Map>::parse_imp( zview_type v, size_type& pos )
{
	if ( pos >= v.size() )
		throw json_exception( "unexpected eof" );

	dbExpects( !stdx::isspace( v[ pos ] ) );

	basic_json result;
	size_type i = pos;

	const auto c = v[ i ];
	switch ( c )
	{
		case 'n': // null
		{
			if ( v.substr( i, 4 ) != "null" )
				throw json_exception( "expected \"null\"" );

			i += 4;
			break;
		}

		case 't': // true
		{
			if ( v.substr( i, 4 ) != "true" )
				throw json_exception( "expected \"true\"" );

			i += 4;
			result = true;
			break;
		}

		case 'f': // false
		{
			if ( v.substr( i, 5 ) != "false" )
				throw json_exception( "expected \"false\"" );

			i += 5;
			result = false;
			break;
		}

		case '[': // array
		{
			result = parse_array( v, i );
			break;
		}

		case '{': // object
		{
			result = parse_object( v, i );
			break;
		}

		case '"': // string
		{
			result = parse_string( v, i );
			break;
		}

		default: // number
		{
			result = parse_number( v, i );
			break;
		}
	}

	while ( stdx::isspace( v[ i ] ) )
		++i;

	pos = i;
	return result;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
basic_json<String, Vector, Map>::array_type basic_json<String, Vector, Map>::parse_array( zview_type v, size_type& pos )
{
	dbExpects( v[ pos ] == '[' );
	size_type i = pos;
	array_type array;
	while ( stdx::isspace( v[ ++i ] ) ) {}
	while ( v[ i ] != ']' )
	{
		array.push_back( parse_imp( v, i ) );

		if ( v[ i ] == ',' )
			while ( stdx::isspace( v[ ++i ] ) ) {}
		else if ( v[ i ] == ']' )
			break;
		else
			throw json_exception( "expected \"]\"" );
	}
	pos = i + 1;
	dbEnsures( v[ pos - 1 ] == ']' );
	return array;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
basic_json<String, Vector, Map>::object_type basic_json<String, Vector, Map>::parse_object( zview_type v, size_type& pos )
{
	dbExpects( v[ pos ] == '{' );
	size_type i = pos;
	object_type result;
	while ( stdx::isspace( v[ ++i ] ) ) {}
	while ( v[ i ] != '}' )
	{
		string_type key = parse_string( v, i );
		while ( stdx::isspace( v[ i ] ) ) ++i;
		if ( v[ i ] != ':' )
			throw json_exception( "expected \":\"" );

		while ( stdx::isspace( v[ i ] ) ) ++i;
		basic_json value = parse_imp( v, i );

		const auto insert_result = result.insert( { std::move( key ), std::move( value ) } );
		if ( !insert_result.second )
			throw json_exception( "duplicate object key" );

		if ( v[ i ] == ',' )
			while ( stdx::isspace( v[ ++i ] ) ) {}
		else if ( v[ i ] == '}' )
			break;
		else
			throw json_exception( "expected \"}\"" );
	}
	pos = i + 1;
	dbEnsures( v[ pos - 1 ] == '}' );
	return result;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
basic_json<String, Vector, Map>::string_type basic_json<String, Vector, Map>::parse_string( zview_type v, size_type& pos )
{
	dbExpects( pos < v.size() );
	dbExpects( v[ pos ] == '"' );

	size_type i = pos + 1;
	string_type result;

	bool escaped = false;
	while ( ( v[ i ] != '"' || escaped ) && v[ i ] != 0 )
	{
		if ( escaped )
		{
			switch ( v[ i ] )
			{
				case 'b': result += '\b'; break;
				case 'f': result += '\f'; break;
				case 'n': result += '\n'; break;
				case 'r': result += '\r'; break;
				case 't': result += '\t'; break;
				case '"': result += '"'; break;
				case '\\': result += '\\'; break;
				default: result += v[ i ]; break;
			}
			escaped = false;
		}
		else if ( v[ i ] == '\\' )
			escaped = true;
		else
			result += v[ i ];

		++i;
	}

	if ( v[ i ] == 0 )
		throw json_exception( "unterminated string" );

	pos = i + 1;
	dbEnsures( v[ pos - 1 ] == '"' );
	return result;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
basic_json<String, Vector, Map>::number_type basic_json<String, Vector, Map>::parse_number( zview_type v, size_type& pos )
{
	dbExpects( pos < v.size() );
	dbExpects( stdx::isdigit( v[ pos ] ) );

	number_type result = 0;
	auto[ last, ec ] = std::from_chars( v.data() + pos, v.data() + v.size(), result );
	if ( ec != std::errc{} )
		throw json_exception( "failed to parse number" );

	dbAssert( last > ( v.data() + pos ) );
	pos = static_cast<size_type>( last - v.data() );
	return result;
}

using json = basic_json<std::string, std::vector, stdx::simple_map>;

namespace literals
{
	json operator "" _json( const char* str )
	{
		return json::parse( str );
	}
}

} // namespace stdx