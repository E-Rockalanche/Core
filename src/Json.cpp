#include "Json.h"

Json::Json( const Json& other ) : m_type{ other.m_type }
{
	switch ( m_type )
	{
		case Type::Number:
			m_number = other.m_number;
			break;

		case Type::Boolean:
			m_boolean = other.m_boolean;
			break;

		case Type::Array:
		case Type::Object:
		case Type::String:
			m_impl = other.m_impl->Clone();
			break;

		case Type::Null:
			break;
	}
}

Json::Json( Json&& other ) noexcept : m_type{ other.m_type }
{
	switch ( m_type )
	{
		case Type::Number:
			m_number = other.m_number;
			break;

		case Type::Boolean:
			m_boolean = other.m_boolean;
			break;

		case Type::Array:
		case Type::Object:
		case Type::String:
			m_impl = std::exchange( other.m_impl, nullptr );
			other.m_type = Type::Null;
			break;

		case Type::Null:
			break;
	}
}

void Json::Destroy() noexcept
{
	switch ( m_type )
	{
		case Type::Array:
		case Type::Object:
		case Type::String:
			delete m_impl;
			break;

		case Type::Null:
		case Type::Number:
		case Type::Boolean:
			break;
	}
}

Json& Json::operator=( const Json& other )
{
	Destroy();
	m_type = other.m_type;
	switch ( m_type )
	{
		case Type::Number:
			m_number = other.m_number;
			break;

		case Type::Boolean:
			m_boolean = other.m_boolean;
			break;

		case Type::Array:
		case Type::Object:
		case Type::String:
			m_impl = other.m_impl->Clone();
			break;

		case Type::Null:
			break;
	}
	return *this;
}

Json& Json::operator=( Json&& other ) noexcept
{
	Destroy();
	m_type = other.m_type;
	switch ( m_type )
	{
		case Type::Number:
			m_number = other.m_number;
			break;

		case Type::Boolean:
			m_boolean = other.m_boolean;
			break;

		case Type::Array:
		case Type::Object:
		case Type::String:
			m_impl = std::exchange( other.m_impl, nullptr );
			other.m_type = Type::Null;
			break;

		case Type::Null:
			break;
	}
	return *this;
}

Json& Json::operator[]( index_type index ) noexcept
{
	CheckType( Type::Array );
	auto& elements = static_cast<ArrayImpl*>( m_impl )->m_elements;

	if ( index >= elements.size() )
		elements.resize( index + 1 );

	return elements[ index ];
}

void Json::push_back( const Json& json )
{
	CheckType( Type::Array );
	static_cast<ArrayImpl*>( m_impl )->m_elements.push_back( json );
}

void Json::push_back( Json&& json )
{
	CheckType( Type::Array );
	static_cast<ArrayImpl*>( m_impl )->m_elements.push_back( std::move( json ) );
}

Json& Json::operator[]( std::string_view key ) noexcept
{
	CheckType( Type::Object );

	auto& pairs = static_cast<ObjectImpl*>( m_impl )->m_pairs;
	auto it = pairs.find( key );

	if ( it != pairs.end() )
		return it->second;

	auto result = pairs.insert( { std::string( key.begin(), key.end() ), Json() } );
	dbAssert( result.second );
	return result.first->second;
}

const Json& Json::operator[]( std::string_view key ) const noexcept
{
	dbAssert( m_type == Type::Object );
	auto& pairs = static_cast<ObjectImpl*>( m_impl )->m_pairs;
	auto it = pairs.find( key );
	dbAssert( it != pairs.end() );
	return it->second;
}

bool Json::insert( std::pair<std::string, Json> pair )
{
	CheckType( Type::Object );
	auto result = static_cast<ObjectImpl*>( m_impl )->m_pairs.insert( std::move( pair ) );
	return result.second;
}

bool operator==( const Json& lhs, const Json& rhs ) noexcept
{
	if ( lhs.m_type != rhs.m_type )
		return false;

	switch ( lhs.m_type )
	{
		case Json::Type::Null:
			return true;

		case Json::Type::Number:
			return lhs.m_number == rhs.m_number;

		case Json::Type::Boolean:
			return lhs.m_boolean == rhs.m_boolean;

		case Json::Type::Array:
			return static_cast<const Json::ArrayImpl*>( lhs.m_impl )->m_elements ==
				static_cast<const Json::ArrayImpl*>( rhs.m_impl )->m_elements;

		case Json::Type::Object:
			return static_cast<const Json::ObjectImpl*>( lhs.m_impl )->m_pairs ==
				static_cast<const Json::ObjectImpl*>( rhs.m_impl )->m_pairs;

		case Json::Type::String:
			return static_cast<const Json::StringImpl*>( lhs.m_impl )->m_string ==
				static_cast<const Json::StringImpl*>( rhs.m_impl )->m_string;
	}

	dbBreak();
	return false;
}

bool operator<( const Json& lhs, const Json& rhs ) noexcept
{
	dbAssert( lhs.m_type == rhs.m_type );

	switch ( lhs.m_type )
	{
		case Json::Type::Null:
			return false;

		case Json::Type::Number:
			return lhs.m_number < rhs.m_number;

		case Json::Type::Boolean:
			return lhs.m_boolean < rhs.m_boolean;

		case Json::Type::Array:
			return static_cast<const Json::ArrayImpl*>( lhs.m_impl )->m_elements <
				static_cast<const Json::ArrayImpl*>( rhs.m_impl )->m_elements;

		case Json::Type::Object:
			return static_cast<const Json::ObjectImpl*>( lhs.m_impl )->m_pairs <
				static_cast<const Json::ObjectImpl*>( rhs.m_impl )->m_pairs;

		case Json::Type::String:
			return static_cast<const Json::StringImpl*>( lhs.m_impl )->m_string <
				static_cast<const Json::StringImpl*>( rhs.m_impl )->m_string;
	}

	dbBreak();
	return false;
}

void Json::SerializeInternal( std::string& json, size_t spaces, size_t depth ) const
{
	switch ( m_type )
	{
		case Type::Number:
		{
			char cbuf[ 256 ];
			std::snprintf( cbuf, std::size( cbuf ), "%f", m_number );
			json += cbuf;
			break;
		}

		case Type::Boolean:
			json += ( m_boolean ? "true" : "false" );
			break;

		case Type::Null:
			json += "null";
			break;

		case Type::String:
		{
			auto& string = static_cast<const StringImpl*>( m_impl )->m_string;
			detail::SerializeString( json, string );
			break;
		}

		case Type::Array:
		{
			json += '[';
			auto& elements = static_cast<const ArrayImpl*>( m_impl )->m_elements;
			for ( size_t i = 0; i < elements.size(); ++i )
			{
				if ( i != 0 )
					json += ',';

				detail::NewLine( json, spaces );
				detail::Indent( json, spaces, depth + 1 );

				elements[ i ].SerializeInternal( json, spaces, depth + 1 );
			}

			detail::NewLine( json, spaces );
			detail::Indent( json, spaces, depth );
			json += ']';
			break;
		}

		case Type::Object:
		{
			json += '{';
			auto& pairs = static_cast<const ObjectImpl*>( m_impl )->m_pairs;
			for ( auto it = pairs.begin(); it != pairs.end(); ++it )
			{
				if ( it != pairs.begin() )
					json += ',';

				detail::NewLine( json, spaces );
				detail::Indent( json, spaces, depth + 1 );

				detail::SerializeString( json, it->first );

				json += ':';
				if ( spaces > 0 )
					json += ' ';

				it->second.SerializeInternal( json, spaces, depth + 1 );
			}

			detail::NewLine( json, spaces );
			detail::Indent( json, spaces, depth );
			json += '}';
			break;
		}
	}
}

Json Json::ParseInternal( std::string_view str, size_t& currentIndex )
{
	Json json;
	size_t index = currentIndex;

	detail::SkipWhitespace( str, index );

	if ( index >= str.size() )
	{
		currentIndex = index;
		throw DeserializeJsonException( "Unexpected end of json string" );
	}

	char c = str[ index ];
	if ( std::isdigit( c ) )
	{
		char* end = nullptr;
		json.m_number = std::strtod( str.data() + index, &end );
		json.m_type = Type::Number;
		index += end - ( str.data() + index );
	}
	else if ( c == 't' )
	{
		if ( index + 4 > str.size() )
		{
			currentIndex = index;
			throw DeserializeJsonException( "Unexpected end of json string" );
		}

		if ( str.substr( index, 4 ) == "true" )
		{
			json = true;
		}
		else
		{
			currentIndex = index;
			throw DeserializeJsonException( "Unexpected true" );
		}
	}
	else if ( c == 'f' )
	{
		if ( index + 5 > str.size() )
		{
			currentIndex = index;
			throw DeserializeJsonException( "Unexpected end of json string" );
		}

		if ( str.substr( index, 5 ) == "false" )
		{
			json = false;
		}
		else
		{
			currentIndex = index;
			throw DeserializeJsonException( "Unexpected false" );
		}
	}
	else if ( c == '\"' )
	{
		json = detail::DeserializeString( str, index );
	}
	else if ( c == '[' )
	{
		++index;
		json = Json::Array();
		bool canReadValue = true;
		while ( true )
		{
			detail::SkipWhitespace( str, index );
			if ( str[ index ] == ']' )
			{
				break;
			}
			else if ( str[ index == ',' ] )
			{
				if ( !canReadValue )
				{
					canReadValue = true;
				}
				else
				{
					currentIndex = index;
					throw DeserializeJsonException( "Missing array element before ','" );
				}
			}
			else
			{
				if ( !canReadValue )
				{
					currentIndex = index;
					throw DeserializeJsonException( "Missing ',' between array elements" );
				}

				json.push_back( ParseInternal( str, index ) );
				canReadValue = false;
			}
		}
		++index;
	}
	else if ( c == '{' )
	{
		++index;
		json = Json::Object();
		bool canReadValue = true;
		while ( true )
		{
			detail::SkipWhitespace( str, index );
			if ( str[ index ] == '}' )
			{
				break;
			}
			else if ( str[ index ] == ',' )
			{
				if ( !canReadValue )
				{
					canReadValue = true;
				}
				else
				{
					currentIndex = index;
					throw DeserializeJsonException( "Missing object property before ','" );
				}
			}
			else
			{
				if ( !canReadValue )
				{
					currentIndex = index;
					throw DeserializeJsonException( "Missing ',' between object properties" );
				}

				if ( str[ index ] != '\"' )
				{
					currentIndex = index;
					throw DeserializeJsonException( "Expected object property name" );
				}

				std::string propertyName = detail::DeserializeString( str, index );

				detail::SkipWhitespace( str, index );

				bool inserted = json.insert( { std::move( propertyName ), ParseInternal( str, index ) } );

				if ( !inserted )
				{
					currentIndex = index;
					throw DeserializeJsonException( "Duplicate object property" );
				}

				canReadValue = false;
			}
		}
		++index;
	}
	currentIndex = index;

	return json;
}