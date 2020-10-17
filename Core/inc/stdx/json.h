#pragma once

#include <stdx/basic_iterator.h>
#include <stdx/ctype.h>
#include <stdx/simple_map.h>
#include <stdx/type_traits.h>
#include <stdx/zstring_view.h>

#include <charconv>
#include <iterator>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace stdx
{

// variant type ordering must match this
enum class json_type
{
	null,
	number,
	boolean,
	string,
	array,
	object
};

namespace detail
{
	template <typename C>
	using key_comp_t = decltype( std::declval<C>().key_comp() );

	template <typename C>
	using key_eq_t = decltype( std::declval<C>().key_eq() );

	template <typename Json>
	class json_index_reference
	{
	public:
		using size_type = typename Json::size_type;
		using char_type = typename Json::char_type;
		using pair_type = typename Json::object_type::value_type;
		using json_type = std::remove_const_t<Json>;

		json_index_reference( Json* json, size_type index ) : m_json{ json }, m_index{ index } {}

		json_index_reference( const json_index_reference& ) = delete;
		json_index_reference& operator=( const json_index_reference& ) = delete;

		operator char_type() const { return m_json->str()[ m_index ]; }
		operator stdx::use_constness_t<Json, char_type>&() const { return m_json->str()[ m_index ]; }

		operator json_type() const { return m_json->arr()[ m_index ]; }
		operator Json&() const { return m_json->arr()[ m_index ]; }

		operator pair_type() const { return *( m_json->items().begin() + m_index ); }
		operator stdx::use_constness_t<Json, pair_type>&() const { return *( m_json->items().begin() + m_index ); }

		json_index_reference& operator=( char_type c ) const
		{
			m_json->str[ m_index ] = c;
			return *this;
		}

		json_index_reference& operator=( const json_type& json ) const
		{
			m_json->arr[ m_index ] = json;
			return *this;
		}

		json_index_reference& operator=( json_type&& json ) const
		{
			m_json->arr[ m_index ] = std::move( json );
			return *this;
		}

	private:
		Json* m_json;
		size_type m_index;
	};

	template <typename Json>
	class json_iterator_cursor
	{
		using object_value_type = typename Json::object_type::value_type;

	public:
		using size_type = typename Json::size_type;
		using difference_type = typename Json::difference_type;
		using reference = json_index_reference<Json>;
		using pointer = std::conditional_t<std::is_const_v<Json>, const object_value_type, object_value_type>*;

		json_iterator_cursor() noexcept = default;

		json_iterator_cursor( Json* json, size_type index ) noexcept
			: m_json{ json }
			, m_index{ index }
#ifdef DEBUG
			, m_type{ json->get_type() }
#endif
		{}

		reference read() const noexcept
		{
			dbExpects( m_json );
			dbExpects( m_json->get_type() == m_type );
			return reference{ m_json, m_index };
		}

		pointer arrow() const noexcept
		{
			dbExpects( m_json );
			dbExpects( m_json->get_type() == m_type );
			return ( m_json->items().begin() + m_index ).operator->();
		}

		void next() noexcept
		{
			dbExpects( m_json );
			dbExpects( m_json->get_type() == m_type );
			dbExpects( m_index < m_json->size() );
			++m_index;
		}

		void prev() noexcept
		{
			dbExpects( m_json );
			dbExpects( m_json->get_type() == m_type );
			dbExpects( m_index > 0 );
			--m_index;
		}

		void advance( difference_type n ) noexcept
		{
			dbExpects( m_json );
			dbExpects( m_json->get_type() == m_type );
			dbExpects( static_cast<difference_type>( m_index ) + n >= 0 );
			dbExpects( m_index + n <= m_json->size() );
			m_index += n;
		}

		void advance( size_type n ) noexcept
		{
			dbExpects( m_json );
			dbExpects( m_json->get_type() == m_type );
			dbExpects( m_index + n <= m_json->size() );
			m_index += n;
		}

		difference_type distance_to( const json_iterator_cursor& other ) const noexcept
		{
			dbExpects( m_json );
			dbExpects( m_json->get_type() == m_type );
			dbExpects( other.m_type == m_type );
			dbExpects( m_json == other.m_json );
			return static_cast<difference_type>( other.m_index ) - static_cast<difference_type>( m_index );
		}

		bool equal( const json_iterator_cursor& other ) const noexcept
		{
			dbExpects( m_json );
			dbExpects( m_json->get_type() == m_type );
			dbExpects( other.m_type == m_type );
			dbExpects( m_json == other.m_json );
			return m_index == other.m_index;
		}

	private:
		Json* m_json = nullptr;
		size_type m_index = 0;

#ifdef DEBUG
		json_type m_type = json_type::null;
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
	static_assert( std::is_class_v<String>, "String must be a class type" );

public:

	using type = json_type;

	struct null_type {};

	using number_type = double;

	using string_type = String;
	using char_type = std::remove_const_t<typename stdx::container_traits<String>::value_type>;
	using view_type = std::basic_string_view<char_type>;
	using zview_type = stdx::basic_zstring_view<char_type>;
	using string_iterator = typename string_type::iterator;
	using string_const_iterator = typename string_type::const_iterator;

	using array_type = Vector<basic_json>;
	using array_iterator = typename array_type::iterator;
	using array_const_iterator = typename array_type::const_iterator;

	using key_type = string_type;
	using mapped_type = basic_json;
	using object_type = Map<key_type, mapped_type>;
	using object_value_type = typename object_type::value_type;
	using object_iterator = typename object_type::iterator;
	using object_const_iterator = typename object_type::const_iterator;

	using reference = detail::json_index_reference<basic_json>;
	using const_reference = detail::json_index_reference<const basic_json>;

	using size_type = std::common_type_t<typename array_type::size_type, typename string_type::size_type, typename object_type::size_type>;
	using difference_type = std::common_type_t<typename array_type::difference_type, typename string_type::difference_type, typename object_type::difference_type>;

	using iterator = stdx::basic_iterator<detail::json_iterator_cursor<basic_json>>;
	using const_iterator = stdx::basic_iterator<detail::json_iterator_cursor<const basic_json>>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static constexpr size_type npos = static_cast<size_type>( -1 );

	// construction

	basic_json() noexcept = default;

	basic_json( const basic_json& other ) = default;

	basic_json( basic_json&& other ) noexcept : m_data{ std::exchange( other.m_data, null_type{} ) } {}

	basic_json( std::nullptr_t ) noexcept {}

	basic_json( number_type value ) noexcept : m_data{ value } {}

	template <typename T,
		std::enable_if_t<stdx::is_numeric_v<T>, int> = 0>
	basic_json( T value ) : m_data{ static_cast<number_type>( value ) } {}

	basic_json( bool value ) noexcept : m_data{ value } {}

	basic_json( const string_type& value ) : m_data{ value } {}

	basic_json( string_type&& value ) noexcept : m_data{ std::move( value ) } {}

	basic_json( view_type value ) : m_data{ string_type( value ) } {}

	basic_json( const char_type* value ) : basic_json( string_type{ value } ) {}
	template <std::size_t N>
	basic_json( const char_type( &value )[ N ] ) : basic_json( string_type{ value } ) {}

	basic_json( const array_type& value ) : m_data{ value } {}

	basic_json( array_type&& value ) noexcept : m_data{ std::move( value ) } {}

	basic_json( std::initializer_list<basic_json> init ) : basic_json( array_type{ init } ) {}

	basic_json( const object_type& value ) : m_data{ value } {}

	basic_json( object_type&& value ) noexcept : m_data{ std::move( value ) } {}

	basic_json( std::initializer_list<std::pair<std::string, basic_json>> init ) : basic_json( object_type{ init } ) {}

	template <typename T,
		std::enable_if_t<std::is_convertible_v<T, view_type>, int> = 0>
	basic_json( T&& t ) : m_data{ string_type( view_type( t ) ) } {}

	template <typename T,
		std::enable_if_t<stdx::is_list_v<T> || stdx::is_array_like_v<T> || stdx::is_set_v<T>, int> = 0>
	basic_json( T&& t ) : m_data{ array_type( std::begin( t ), std::end( t ) ) } {}

	template <typename T,
		std::enable_if_t<stdx::is_map_v<T>, int> = 0>
	basic_json( T&& t ) : m_data{ object_type( std::begin( t ), std::end( t ) ) } {}

	static basic_json null() { return basic_json{}; }
	static basic_json boolean( bool value = false ) { return basic_json{ value }; }
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
	static basic_json object( std::initializer_list<typename object_value_type> init ) { return basic_json{ init }; }

	// assignment

	basic_json& operator=( const basic_json& other ) = default;

	basic_json& operator=( basic_json&& other ) noexcept = default;

	// conversion

	number_type& num() { return std::get<number_type>( m_data ); }
	const number_type& num() const { return std::get<number_type>( m_data ); }

	string_type& str() { return std::get<string_type>( m_data ); }
	const string_type& str() const { return std::get<string_type>( m_data ); }

	array_type& arr() { return std::get<array_type>( m_data ); }
	const array_type& arr() const { return std::get<array_type>( m_data ); }

	object_type& items() { return std::get<object_type>( m_data ); }
	const object_type& items() const { return std::get<object_type>( m_data ); }

	operator bool() const noexcept
	{
		switch ( get_type() )
		{
			case type::null:	return false;
			case type::number:	return std::get<number_type>( m_data ) > 0;
			case type::boolean: return std::get<bool>( m_data );
			case type::string:	return !std::get<string_type>( m_data ).empty();
			case type::array:	return !std::get<array_type>( m_data ).empty();
			case type::object:	return !std::get<object_type>( m_data ).empty();
		}
		dbBreak();
		return false;
	}

	operator view_type() const
	{
		if ( get_type() != type::string )
			throw_exception( "cannot convert to string view" );

		return str();
	}

	template <typename T,
		std::enable_if_t<stdx::is_numeric_v<T>, int> = 0>
	operator T() const
	{
		switch ( get_type() )
		{
			case type::number:	return static_cast<T>( std::get<number_type>( m_data ) );
			case type::boolean:	return static_cast<T>( std::get<bool>( m_data ) );
			default: throw_exception( "cannot convert to numeric type" );
		}
	}

	template<typename T,
		std::enable_if_t<std::is_convertible_v<T, view_type>, int> = 0>
	operator T() const
	{
		static_assert( !std::is_pointer_v<T>, "use c_str()" );

		if ( !is_string() )
			throw_exception( "cannot convert to string type" );

		return T( std::get<string_type>( m_data ) );
	}

	template <typename T>
	T get() const&;

	template <>
	bool get<bool>() const& { return std::get<bool>( m_data ); }

	template <>
	number_type get<number_type>() const& { return std::get<number_type>( m_data ); }

	template <>
	string_type get<string_type>() const& { return std::get<string_type>( m_data ); }

	template <>
	array_type get<array_type>() const& { return std::get<array_type>( m_data ); }

	template <>
	object_type get<object_type>() const& { return std::get<object_type>( m_data ); }

	template <typename T>
	T get() &&;

	template <>
	bool get<bool>() && { return std::get<bool>( m_data ); }

	template <>
	number_type get<number_type>() && { return std::get<number_type>( m_data ); }

	template <>
	string_type get<string_type>() && { return std::move( std::get<string_type>( m_data ) ); }

	template <>
	array_type get<array_type>() && { return std::move( std::get<array_type>( m_data ) ); }

	template <>
	object_type get<object_type>() && { return std::move( std::get<object_type>( m_data ) ); }

	template <typename T>
	operator T() const& { return get<T>(); }

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

	iterator begin() noexcept
	{
		dbExpects( is_array() || is_string() || is_object() );
		return iterator( this, 0 );
	}

	iterator end() noexcept
	{
		dbExpects( is_array() || is_string() || is_object() );
		return iterator( this, size() );
	}

	const_iterator begin() const noexcept
	{
		dbExpects( is_array() || is_string() || is_object() );
		return const_iterator( this, 0 );
	}

	const_iterator end() const noexcept
	{
		dbExpects( is_array() || is_string() || is_object() );
		return const_iterator( this, size() );
	}

	const_iterator cbegin() const noexcept { return begin(); }
	const_iterator cend() const noexcept { return end(); }

	reverse_iterator rbegin() noexcept { return reverse_iterator( end() ); }
	reverse_iterator rend() noexcept { return reverse_iterator( begin() ); }

	const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator( end() ); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator( begin() ); }

	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator( end() ); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator( begin() ); }

	// shared access

	reference at( size_type index )
	{
		if ( index >= size() )
			throw_exception( "index out of bounds" );

		return reference( this, index );
	}

	const_reference at( size_type index ) const
	{
		if ( index >= size() )
			throw_exception( "index out of bounds" );

		return const_reference( this, index );
	}

	reference operator[]( size_type index ) noexcept { return reference( this, index ); }
	const_reference operator[]( size_type index ) const noexcept { return reference( this, index ); }

	reference front() noexcept
	{
		dbExpects( !empty() );
		return operator[]( 0 );
	}

	const_reference front() const noexcept
	{
		dbExpects( !empty() );
		return operator[]( 0 );
	}

	reference back()
	{
		dbExpects( !empty() );
		return operator[]( size() - 1 );
	}

	const_reference back() const
	{
		dbExpects( !empty() );
		return operator[]( size() - 1 );
	}

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

			default: throw_exception( "current type has no empty()" );
		}
	}

	size_type size() const
	{
		switch ( get_type() )
		{
			case type::string: return str().size();
			case type::array: return arr().size();
			case type::object: return items().size();

			default: throw_exception( "current type has no size()" );
		}
	}

	size_type max_size() const
	{
		switch ( get_type() )
		{
			case type::string: return str().max_size();
			case type::array: return arr().max_size();
			case type::object: return items().max_size();

			default: throw_exception( "current type has no max_size()" );
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

			default: throw_exception( "current type has no reserve(size_type)" );
		}
	}

	size_type capacity() const
	{
		switch ( get_type() )
		{
			case type::string: return str().capacity();
			case type::array: return arr().capacity();
			case type::object: return items().capacity();

			default: throw_exception( "current type has no capacity()" );
		}
	}

	void shrink_to_fit()
	{
		switch ( get_type() )
		{
			case type::string: return str().shrink_to_fit();
			case type::array: return arr().shrink_to_fit();
			case type::object: return items().shrink_to_fit();

			default: throw_exception( "current type has no shrink_to_fit()" );
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

			default: throw_exception( "current type has no clear()" );
		}
	}

	void resize( size_type count )
	{
		switch ( get_type() )
		{
			case type::string: str().resize( count ); break;
			case type::array: arr().resize( count ); break;

			default: throw_exception( "current type has no resize(size_type)" );
		}
	}

	template <typename T>
	void resize( size_type count, const T& value )
	{
		switch ( get_type() )
		{
			case type::string: str().resize( count, value ); break;
			case type::array: arr().resize( count, value ); break;

			default: throw_exception( "current type has no resize(size_type, const T&)" );
		}
	}

	void swap( basic_json&& other )
	{
		auto temp = std::move( m_data );
		m_data = std::move( other.m_data );
		other.m_data = std::move( temp );
	}

	// string operators

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
	basic_json& replace( string_const_iterator first, string_const_iterator last, const string_type& s ) { str().replace( first, last, s ); return *this; }
	basic_json& replace( size_type pos, size_type count, const string_type& s, size_type pos2, size_type count2 = npos ) { str().replace( pos, count, s, pos2, count2 ); return *this; }
	template <typename InputIt>
	basic_json& replace( string_const_iterator first, string_const_iterator last, InputIt first2, InputIt last2 ) { str().replace( first, last, first2, last2 ); return *this; }
	basic_json& replace( size_type pos, size_type count, const char_type* s, size_type count2 ) { str().replace( pos, count, s, count2 ); return *this; }
	basic_json& replace( string_const_iterator first, string_const_iterator last, const char_type* s, size_type count ) { str().replace( first, last, s, count ); return *this; }
	basic_json& replace( size_type pos, size_type count, const char_type* s ) { str().replace( pos, count, s ); return *this; }
	basic_json& replace( string_const_iterator first, string_const_iterator last, const char_type* s ) { str().replace( first, last, s ); return *this; }
	basic_json& replace( size_type pos, size_type count, size_type count2, char_type c ) { str().replace( pos, count, count2, c ); return *this; }
	basic_json& replace( string_const_iterator first, string_const_iterator last, size_type count2, char_type c ) { str().replace( first, last, count2, c ); return *this; }
	basic_json& replace( string_const_iterator first, string_const_iterator last, std::initializer_list<char_type> init ) { str().replace( first, last, init ); return *this; }
	template <typename T>
	basic_json& replace( size_type pos, size_type count, const T& t ) { str().replace( pos, count, t ); return *this; }
	template <typename T>
	basic_json& replace( string_const_iterator first, string_const_iterator last, const T& t ) { str().replace( first, last, t ); return *this; }
	template <typename T>
	basic_json& replace( size_type pos, size_type count, const T& t, size_type pos2, size_type count2 = npos ) { str().replace( pos, count, t, pos2, count2 ); return *this; }
	basic_json& replace( size_type pos, size_type count, const basic_json& j ) { str().replace( pos, count, std::get<string_type>( j.m_data ) ); return *this; }
	basic_json& replace( string_const_iterator first, string_const_iterator last, const basic_json& j ) { str().replace( first, last, std::get<string_type>( j.m_data ) ); return *this; }
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

	void insert( array_const_iterator pos, const basic_json& j ) { arr().insert( pos, j ); }
	void insert( array_const_iterator pos, basic_json&& j ) { arr().insert( pos, std::move( j ) ); }
	void insert( array_const_iterator pos, size_type count, const basic_json& j ) { arr().insert( pos, count, j ); }
	template <typename InputIt>
	void insert( array_const_iterator pos, InputIt first, InputIt last ) { arr().insert( pos, first, last ); }
	void insert( array_const_iterator pos, std::initializer_list<basic_json> init ) { arr().insert( pos, init ); }

	template <typename... Args>
	void emplace( array_const_iterator pos, Args... args ) { arr().emplace( pos, std::forward<Args>( args )... ); }

	array_iterator erase( array_const_iterator pos ) { return arr().erase( pos ); }
	array_iterator erase( array_const_iterator first, array_const_iterator last ) { return arr().erase( first, last ); }

	void push_back( const basic_json& j ) { return arr().push_back( j ); }
	void push_back( basic_json&& j ) { return arr().push_back( std::move( j ) ); }

	template <typename... Args>
	void emplace_back( Args&&... args ) { return arr().emplace_back( std::forward<Args>( args )... ); }

	void pop_back() { arr().pop_back(); }

	// object operators

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

	std::pair<object_iterator, bool> insert( const object_value_type& pair ) { return items().insert( pair ); }
	std::pair<object_iterator, bool> insert( object_value_type&& pair ) { return items().insert( std::move( pair ) ); }

	template <typename M>
	std::pair<object_iterator, bool> insert_or_assign( const key_type& k, M&& obj ) { return items().insert_or_assign( k, std::forward<M>( obj ) ); }
	template <typename M>
	std::pair<object_iterator, bool> insert_or_assign( key_type&& k, M&& obj ) { return items().insert_or_assign( std::move( k ), std::forward<M>( obj ) ); }

	template <typename... Args>
	std::pair<object_iterator, bool> emplace( Args&&... args ) { return items().emplace( std::forward<Args>( args )... ); }

	template <typename... Args>
	std::pair<object_iterator, bool> try_emplace( const key_type& k, Args&&... args ) { return items().try_emplace( k, std::forward<Args>( args )... ); }
	template <typename... Args>
	std::pair<object_iterator, bool> try_emplace( key_type&& k, Args&&... args ) { return items().try_emplace( std::move( k ), std::forward<Args>( args )... ); }

	object_iterator erase( object_iterator pos ) { return items().erase( pos ); }
	object_iterator erase( object_const_iterator pos ) { return items().erase( pos ); }
	object_iterator erase( object_const_iterator first, object_const_iterator last ) { return items().erase( first, last ); }
	size_type erase( const key_type k ) { return items().erase( k ); }

	object_iterator find( const key_type& k ) { return items().find( k ); }
	object_const_iterator find( const key_type& k ) const { return items().find( k ); }
	template <typename K>
	object_iterator find( const K& k ) { return items().find( k ); }
	template <typename K>
	object_const_iterator find( const K& k ) const { return items().find( k ); }

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

	template <typename Obj = object_type, std::enable_if_t<stdx::is_detected_v<detail::key_comp_t, Obj>, int> = 0>
	auto key_comp() const { return items().key_comp(); }

	template <typename Obj = object_type, std::enable_if_t<stdx::is_detected_v<detail::key_eq_t, Obj>, int> = 0>
	auto key_eq() const { return items().key_eq(); }

	// non member functions

	template <typename String, template<class> typename Vector, template<class, class> typename Map>
	friend bool operator==( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs );

	template <typename String, template<class> typename Vector, template<class, class> typename Map>
	friend bool operator!=( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs );

	template <typename String, template<class> typename Vector, template<class, class> typename Map>
	friend bool operator<( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs );

	template <typename String, template<class> typename Vector, template<class, class> typename Map>
	friend bool operator>( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs );

	template <typename String, template<class> typename Vector, template<class, class> typename Map>
	friend bool operator<=( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs );

	template <typename String, template<class> typename Vector, template<class, class> typename Map>
	friend bool operator>=( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs );

	template <typename String, template<class> typename Vector, template<class, class> typename Map>
	friend std::ostream& operator<<( std::ostream& out, const basic_json<String, Vector, Map>& json );

private:
	template<typename Error>
	static void throw_exception( Error error )
	{
		dbBreak();
		throw json_exception( error );
	}

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
T basic_json<String, Vector, Map>::get() const&
{
	if constexpr ( stdx::is_numeric_v<T> )
	{
		return static_cast<T>( std::get<number_type>( m_data ) );
	}
	else if constexpr ( std::is_convertible_v<T, view_type> )
	{
		return view_type( str() );
	}
	else if constexpr ( stdx::is_list_v<T> || stdx::is_set_v<T> )
	{
		auto& array = arr();
		return T( array.begin(), array.end() );
	}
	else if constexpr ( stdx::is_map_v<T> )
	{
		auto& object = items();
		return T( object.begin(), object.end() );
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
	if constexpr ( stdx::is_numeric_v<T> )
	{
		return static_cast<T>( std::get<number_type>( m_data ) );
	}
	else if constexpr ( std::is_convertible_v<T, view_type> )
	{
		return view_type( str() );
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
	using json = basic_json<String, Vector, Map>;

	const auto type = lhs.get_type();
	if ( type != rhs.get_type() )
		throw json_exception( "basic_json type compare mismatch" );

	switch ( type )
	{
		case json_type::null:		return true;
		case json_type::boolean:	return std::get<bool>( lhs.m_data ) == std::get<bool>( rhs.m_data );
		case json_type::number:		return std::get<typename json::number_type>( lhs.m_data ) == std::get<typename json::number_type>( rhs.m_data );
		case json_type::string:		return std::get<typename json::string_type>( lhs.m_data ) == std::get<typename json::string_type>( rhs.m_data );
		case json_type::array:		return std::get<typename json::array_type>( lhs.m_data ) == std::get<typename json::array_type>( rhs.m_data );
		case json_type::object:		return std::get<typename json::object_type>( lhs.m_data ) == std::get<typename json::object_type>( rhs.m_data );
	}

	dbBreak();
	return false;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
bool operator!=( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs )
{
	return !( lhs == rhs );
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
bool operator<( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs )
{
	using json = basic_json<String, Vector, Map>;

	const auto type = lhs.get_type();
	if ( type != rhs.get_type() )
		throw json_exception( "basic_json type compare mismatch" );

	switch ( type )
	{
		case json_type::null:		return false;
		case json_type::boolean:	return std::get<bool>( lhs.m_data ) < std::get<bool>( rhs.m_data );
		case json_type::number:		return std::get<typename json::number_type>( lhs.m_data ) < std::get<typename json::number_type>( rhs.m_data );
		case json_type::string:		return std::get<typename json::string_type>( lhs.m_data ) < std::get<typename json::string_type>( rhs.m_data );
		case json_type::array:		return std::get<typename json::array_type>( lhs.m_data ) < std::get<typename json::array_type>( rhs.m_data );
		case json_type::object:		return std::get<typename json::object_type>( lhs.m_data ) < std::get<typename json::object_type>( rhs.m_data );
	}

	dbBreak();
	return false;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
bool operator>( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs )
{
	return rhs < lhs;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
bool operator<=( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs )
{
	return !( lhs > rhs );
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
bool operator>=( const basic_json<String, Vector, Map>& lhs, const basic_json<String, Vector, Map>& rhs )
{
	return !( lhs < rhs );
}

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
			case '\b':	s += "\\b";		break;
			case '\f':	s += "\\f";		break;
			case '\n':	s += "\\n";		break;
			case '\r':	s += "\\r";		break;
			case '\t':	s += "\\t";		break;
			case '"':	s += "\\\"";	break;
			case '\\':	s += "\\\\";	break;
			default:	s += c;			break;
		}
	}
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
basic_json<String, Vector, Map> basic_json<String, Vector, Map>::parse_imp( zview_type v, size_type& pos )
{
	if ( pos >= v.size() )
		throw_exception( "unexpected eof" );

	dbExpects( !stdx::isspace( v[ pos ] ) );

	basic_json result;
	size_type i = pos;

	const auto c = v[ i ];
	switch ( c )
	{
		case 'n': // null
		{
			if ( v.substr( i, 4 ) != "null" )
				throw_exception( "expected \"null\"" );

			i += 4;
			break;
		}

		case 't': // true
		{
			if ( v.substr( i, 4 ) != "true" )
				throw_exception( "expected \"true\"" );

			i += 4;
			result = true;
			break;
		}

		case 'f': // false
		{
			if ( v.substr( i, 5 ) != "false" )
				throw_exception( "expected \"false\"" );

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
typename basic_json<String, Vector, Map>::array_type basic_json<String, Vector, Map>::parse_array( zview_type v, size_type& pos )
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
			throw_exception( "expected \"]\"" );
	}
	pos = i + 1;
	dbEnsures( v[ pos - 1 ] == ']' );
	return array;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
typename basic_json<String, Vector, Map>::object_type basic_json<String, Vector, Map>::parse_object( zview_type v, size_type& pos )
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
			throw_exception( "expected \":\"" );

		while ( stdx::isspace( v[ i ] ) ) ++i;
		basic_json value = parse_imp( v, i );

		const auto insert_result = result.insert( { std::move( key ), std::move( value ) } );
		if ( !insert_result.second )
			throw_exception( "duplicate object key" );

		if ( v[ i ] == ',' )
			while ( stdx::isspace( v[ ++i ] ) ) {}
		else if ( v[ i ] == '}' )
			break;
		else
			throw_exception( "expected \"}\"" );
	}
	pos = i + 1;
	dbEnsures( v[ pos - 1 ] == '}' );
	return result;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
typename basic_json<String, Vector, Map>::string_type basic_json<String, Vector, Map>::parse_string( zview_type v, size_type& pos )
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
		throw_exception( "unterminated string" );

	pos = i + 1;
	dbEnsures( v[ pos - 1 ] == '"' );
	return result;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
typename basic_json<String, Vector, Map>::number_type basic_json<String, Vector, Map>::parse_number( zview_type v, size_type& pos )
{
	dbExpects( pos < v.size() );
	dbExpects( stdx::isdigit( v[ pos ] ) );

	number_type result = 0;
	auto[ last, ec ] = std::from_chars( v.data() + pos, v.data() + v.size(), result );
	if ( ec != std::errc{} )
		throw_exception( "failed to parse number" );

	dbAssert( last > ( v.data() + pos ) );
	pos = static_cast<size_type>( last - v.data() );
	return result;
}

template <typename String, template<class> typename Vector, template<class, class> typename Map>
std::ostream& operator<<( std::ostream& out, const basic_json<String, Vector, Map>& json )
{
	switch ( json.get_type() )
	{
		case json_type::null:
			out << "null";
			return out;

		case json_type::number:
			out << json.num();
			return out;

		case json_type::boolean:
			out << ( json.get<bool>() ? "true" : "false" );
			return out;

		case json_type::string:
			out << '"' << json.str() << '"';
			return out;

		case json_type::array:
		{
			out << '[';
			bool first = true;
			for ( auto& element : json.arr() )
			{
				if ( !first ) out << ',';
				else first = false;

				out << element;
			}
			out << ']';
			return out;
		}

		case json_type::object:
		{
			out << '{';
			bool first = true;
			for ( auto&[ key, value ] : json.items() )
			{
				if ( !first ) out << ',';
				else first = false;

				out << '"' << key << "\":" << value;
			}
			out << '}';
			return out;
		}
	}
	dbBreak();
	return out;
}

using json = basic_json<std::string, std::vector, stdx::simple_map>;

namespace literals
{
	inline json operator "" _json( const char* str )
	{
		return json::parse( str );
	}
}

} // namespace stdx