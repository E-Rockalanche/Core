#pragma once

#include <stdx/format.h>
#include <stdx/memory.h>
#include <stdx/simple_map.h>
#include <stdx/type_traits.h>

#include <cstdio>
#include <cctype>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

class DeserializeJsonException : public std::exception
{
public:
	using std::exception::exception;
};

namespace detail
{
	template <typename C>
	using mapped_type_t = typename C::mapped_type;

	inline void Indent( std::string& json, size_t spaces, size_t depth )
	{
		json.append( spaces * depth, ' ' );
	}

	inline void NewLine( std::string& json, size_t spaces )
	{
		if ( spaces > 0 )
			json += '\n';
	}

	inline void SerializeString( std::string& json, std::string_view str )
	{
		json += '\"';
		for ( char c : str )
		{
			switch ( c )
			{
				case '\b':
				case '\f':
				case '\n':
				case '\r':
				case '\t':
				case '\"':
				case '\\':
					json += '\\';
					break;

				default:
					break;
			}
			json += c;
		}
		json += '\"';
	}

	inline std::string DeserializeString( std::string_view json, size_t& currentIndex )
	{
		std::string str;

		size_t index = currentIndex;
		if ( json[ index ] != '\"' )
			throw DeserializeJsonException( "Espected \" to start string" );

		++index;
		bool escaped = false;
		char c = 0;
		while ( !escaped && ( c = json[ index ] ) != '\"' && index < json.size() )
		{
			if ( c == '\\' )
			{
				escaped = true;
			}
			else if ( escaped )
			{
				escaped = false;
				switch ( c )
				{
					case 'b': str += '\b'; break;
					case 'f': str += '\f'; break;
					case 'n': str += '\n'; break;
					case 'r': str += '\r'; break;
					case 't': str += '\t'; break;
					default: str += c; break;
				}
			}
			else
			{
				str += c;
			}
			++index;
		}

		if ( json[ index ] != '\"' )
		{
			currentIndex = index;
			throw DeserializeJsonException( "Unexpected end of string" );
		}

		++index; // skip " terminator
		currentIndex = index;

		return str;
	}

	void SkipWhitespace( std::string_view str, size_t& currentIndex ) noexcept
	{
		size_t index = currentIndex;

		while ( index < str.size() && std::isspace( str[ index ] ) )
			++index;

		currentIndex = index;
	}
}

class Json
{
public:
	using index_type = std::size_t;

	enum class Type
	{
		Number,
		Boolean,
		Null,
		Object,
		Array,
		String
	};

	// construction

	Json() noexcept : m_type{ Type::Null } {}

	Json( const Json& other );

	Json( Json&& other ) noexcept;

	Json( std::string_view value );

	Json( std::string&& value ) noexcept;

	Json( double value ) noexcept : m_number{ value }, m_type{ Type::Number } {}

	Json( bool value ) noexcept : m_boolean{ value }, m_type{ Type::Boolean } {}

	Json( std::nullptr_t ) noexcept : m_type{ Type::Null } {}

	Json( std::initializer_list<Json> init );

	Json( std::initializer_list<std::pair<std::string, Json>> init );

	template <typename C>
	Json( const C& container );

	~Json() { Destroy(); }

	// assignment

	Json& operator=( const Json& other );

	Json& operator=( Json&& other ) noexcept;

	Json& operator=( std::string_view value )
	{
		return operator=( Json( value ) );
	}

	Json& operator=( std::string&& value ) noexcept
	{
		return operator=( Json( std::move( value ) ) );
	}

	Json& operator=( double value ) noexcept
	{
		Destroy();
		m_number = value;
		m_type = Type::Number;
		return *this;
	}

	Json& operator=( bool value ) noexcept
	{
		Destroy();
		m_boolean = value;
		m_type = Type::Boolean;
		return *this;
	}

	Json& operator=( std::nullptr_t ) noexcept
	{
		Destroy();
		m_type = Type::Null;
		return *this;
	}

	Json& operator=( std::initializer_list<Json> init )
	{
		return operator=( Json( init ) );
	}

	Json& operator=( std::initializer_list<std::pair<std::string, Json>> init )
	{
		return operator=( Json( init ) );
	}

	// queries

	Type GetType() const noexcept { return m_type; }

	bool IsNumber() const noexcept { return m_type == Type::Number; }
	bool IsBoolean() const noexcept { return m_type == Type::Boolean; }
	bool IsNull() const noexcept { return m_type == Type::Null; }
	bool IsString() const noexcept { return m_type == Type::String; }
	bool IsObject() const noexcept { return m_type == Type::Object; }
	bool IsArray() const noexcept { return m_type == Type::Array; }

	// array operators

	Json& operator[]( index_type index ) noexcept;
	const Json& operator[]( index_type index ) const noexcept;

	void push_back( const Json& json );
	void push_back( Json&& json );

	// object operators

	Json& operator[]( std::string_view key ) noexcept;
	const Json& operator[]( std::string_view key ) const noexcept;

	bool insert( std::pair<std::string, Json> pair );

	template <typename T>
	bool insert_or_assign( std::string key, T value );

	// conversion

	template <typename T>
	const T& Get() const;

	static Json Array();
	static Json Array( std::initializer_list<Json> init );

	static Json Object();
	static Json Object( std::initializer_list<std::pair<std::string, Json>> init );

	// comparison

	friend bool operator==( const Json& lhs, const Json& rhs ) noexcept;
	friend bool operator!=( const Json& lhs, const Json& rhs ) noexcept;

	friend bool operator<( const Json& lhs, const Json& rhs ) noexcept;
	friend bool operator>( const Json& lhs, const Json& rhs ) noexcept;
	friend bool operator<=( const Json& lhs, const Json& rhs ) noexcept;
	friend bool operator>=( const Json& lhs, const Json& rhs ) noexcept;

	std::string Serialize( size_t spaces = 0 ) const;

	static Json Parse( std::string_view str );

private:

	struct Impl
	{
		virtual ~Impl() = default;
		virtual Impl* Clone() const = 0;
	};

	struct StringImpl;
	struct ArrayImpl;
	struct ObjectImpl;

	Json( Type type, Impl* impl ) : m_impl{ impl }, m_type{ type } {}

	void Destroy() noexcept;

	void SerializeInternal( std::string& stream, size_t spaces, size_t depth ) const;

	static Json ParseInternal( std::string_view str, size_t& index );

	void CheckType( Type type )
	{
		if ( m_type == Type::Null )
			m_type = type;

		dbAssert( m_type == type );
	}

private:

	union
	{
		double m_number;
		bool m_boolean;
		Impl* m_impl;
	};

	Type m_type;
};

struct Json::StringImpl : Json::Impl
{
	StringImpl() = default;
	StringImpl( const StringImpl& ) = default;
	StringImpl( std::string str ) : m_string{ std::move( str ) } {}

	~StringImpl() override = default;

	StringImpl* Clone() const override { return new StringImpl( *this ); }

	std::string m_string;
};

struct Json::ArrayImpl : Json::Impl
{
	ArrayImpl() = default;
	ArrayImpl( const ArrayImpl& ) = default;
	ArrayImpl( std::vector<Json> elements ) : m_elements{ std::move( elements ) } {}

	~ArrayImpl() override = default;

	ArrayImpl* Clone() const override { return new ArrayImpl( *this ); }

	std::vector<Json> m_elements;
};

struct Json::ObjectImpl : Json::Impl
{
	ObjectImpl() = default;
	ObjectImpl( const ObjectImpl& ) = default;
	ObjectImpl( stdx::simple_map<std::string, Json> pairs ) : m_pairs{ std::move( pairs ) } {}

	~ObjectImpl() override = default;

	ObjectImpl* Clone() const override { return new ObjectImpl( *this ); }

	stdx::simple_map<std::string, Json> m_pairs;
};

Json::Json( std::string_view value )
	: m_impl{ new StringImpl( std::string( value.begin(), value.end() ) ) }
	, m_type { Type::String }
{}

Json::Json( std::string&& value ) noexcept
	: m_impl{ new StringImpl( std::move( value ) ) }
	, m_type{ Type::String }
{}

Json::Json( std::initializer_list<Json> init )
	: m_impl{ new ArrayImpl( init ) }
	, m_type{ Type::Array }
{}

Json::Json( std::initializer_list<std::pair<std::string, Json>> init )
	: m_impl{ new ObjectImpl( init ) }
	, m_type{ Type::Object }
{}

template <typename C>
Json::Json( const C& container )
{
	if constexpr ( std::is_convertible_v<C, std::string_view> )
	{
		m_type = Type::String;
		m_impl = new StringImpl( std::string_view( container ) );
	}
	else if constexpr ( stdx::is_detected_v<detail::mapped_type_t, C> )
	{
		m_type = Type::Object;
		auto obj = std::make_unique<ObjectImpl>();
		for ( auto& pair : container )
		{
			obj->m_pairs.insert( { std::string( pair.first ), Json( pair.second ) } );
		}
		m_impl = obj.release();
	}
	else
	{
		m_type = Type::Array;
		auto arr = std::make_unique<ArrayImpl>();
		for ( auto& value : container )
		{
			arr->m_elements.emplace_back( value );
		}
		m_impl = arr.release();
	}
}

inline const Json& Json::operator[]( index_type index ) const noexcept
{
	dbAssert( m_type == Type::Array );
	return static_cast<ArrayImpl*>( m_impl )->m_elements[ index ];
}

template <typename T>
inline bool Json::insert_or_assign( std::string key, T value )
{
	CheckType( Type::Object );
	auto result = static_cast<ObjectImpl*>( m_impl )->m_pairs.insert_or_assign( std::move( key ), std::move( value ) );
	return result.second;
}

template<>
inline const std::string& Json::Get<std::string>() const
{
	dbAssert( m_type == Type::String );
	return static_cast<StringImpl*>( m_impl )->m_string;
}

template<>
inline const bool& Json::Get<bool>() const
{
	dbAssert( m_type == Type::Boolean );
	return m_boolean;
}

template<>
inline const double& Json::Get<double>() const
{
	dbAssert( m_type == Type::Number );
	return m_number;
}

template <typename T>
inline const T& Json::Get() const
{
	static_assert( "Can only explicitely convert to bool, double, or std::string" );
}

inline bool operator!=( const Json& lhs, const Json& rhs ) noexcept { return !( lhs == rhs ); }

inline bool operator>( const Json& lhs, const Json& rhs ) noexcept { return rhs < lhs; }

inline bool operator<=( const Json& lhs, const Json& rhs ) noexcept { return !( lhs > rhs ); }

inline bool operator>=( const Json& lhs, const Json& rhs ) noexcept { return !( lhs < rhs ); }

inline std::string Json::Serialize( size_t spaces ) const
{
	std::string stream;
	SerializeInternal( stream, spaces, 0 );
	return stream;
}

inline Json Json::Parse( std::string_view str )
{
	size_t index = 0;
	return ParseInternal( str, index );
}

inline Json Json::Array()
{
	return Json( Type::Array, new ArrayImpl() );
}

inline Json Json::Array( std::initializer_list<Json> init )
{
	return Json( Type::Array, new ArrayImpl( init ) );
}

inline Json Json::Object()
{
	return Json( Type::Object, new ObjectImpl() );
}

inline Json Json::Object( std::initializer_list<std::pair<std::string, Json>> init )
{
	return Json( Type::Object, new ObjectImpl( init ) );
}