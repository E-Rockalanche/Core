#pragma once

#include "MetaIO.h"
#include "MetaType.h"

#include <stdx/format.h>

#include <cctype>
#include <charconv>
#include <stdio.h>
#include <sstream>

namespace Meta
{

class MetaJsonWriter : public MetaWriter
{
public:
	MetaJsonWriter( std::ostream& o ) : m_ostream( o )
	{
		m_ostream << std::showbase;
	}

	void writeInt( int64_t value, int base = 10 ) override
	{
		switch ( base )
		{
			case 8: m_ostream << std::oct << value; break;
			case 10: m_ostream << std::dec << value; break;
			case 16: m_ostream << std::hex << value; break;
			default: throw MetaIOException( stdx::format( "Invalid integer base: {}", base ) );
		}
	}

	void writeFloat( double value ) override { m_ostream << value; }

	void writeBool( bool value ) override { m_ostream << ( value ? "true" : "false" ); }

	void writeString( std::string_view str ) override
	{
		m_ostream << '"';
		for ( auto c : str )
		{
			switch ( c )
			{
				case '"': m_ostream << "\\\""; break;
				case '\\': m_ostream << "\\\\"; break;
				case '\n': m_ostream << "\\n"; break;
				case '\f': m_ostream << "\\f"; break;
				case '\r': m_ostream << "\\r"; break;
				case '\t': m_ostream << "\\t"; break;
				case '\b': m_ostream << "\\b"; break;
				default: m_ostream << c; break;
			}
		}
		m_ostream << '"';
	}

	void startArray() override { m_ostream << '['; }
	void delimitArray() override { m_ostream << ','; }
	void endArray() override { m_ostream << ']'; }

	void startObject() override { m_ostream << '{'; }
	void delimitObject() override { m_ostream << ','; }
	void endObject() override { m_ostream << '}'; }

	void startVariable( std::string_view name ) override
	{
		writeString( name );
		m_ostream << ':';
	}
	void endVariable() override {}

	void writeNull() override { m_ostream << "null"; }

private:
	std::ostream& m_ostream;
};

class MetaJsonReader : public MetaReader
{
public:
	MetaJsonReader( std::istream& in )
	{
		std::stringstream ss;
		ss << in.rdbuf();
		m_data = ss.str();
	}

	int64_t readInt() override
	{
		skipWhitespace();

		const bool negative = m_data[ m_pos ] == '-';
		const bool hasSign = negative || m_data[ m_pos ] == '+';
		if ( hasSign )
			m_pos++;

		const int base = readIntPrefix();

		const size_t startPos = m_pos;
		while ( std::isalnum( m_data[ m_pos ] ) )
			m_pos++;

		if ( m_pos == startPos && base == 8 )
			return 0;

		int64_t value = 0;
		const char* last = m_data.data() + m_pos;
		auto result = std::from_chars( m_data.data() + startPos, last, value, base );
		if ( result.ptr != last || result.ec != std::errc() )
			throw MetaIOException( stdx::format( "Invalid int64 at pos {}", startPos ) );

		return negative ? -value : value;
	}

	double readFloat() override
	{
		skipWhitespace();
		const size_t startPos = m_pos;
		while ( std::isalnum( m_data[ m_pos ] ) || m_data[ m_pos ] == '.' )
			m_pos++;

		double value = 0.0;
		const char* last = m_data.data() + m_pos;
		auto result = std::from_chars( m_data.data() + startPos, last, value );
		if ( result.ptr != last || result.ec != std::errc() )
			throw MetaIOException( stdx::format( "Invalid double at pos {}", startPos ) );

		return value;
	}

	bool readBool() override
	{
		skipWhitespace();
		auto truefalse = std::string_view( m_data ).substr( m_pos, 5 );
		if ( truefalse == "false" )
		{
			m_pos += 5;
			return false;
		}

		if ( truefalse.substr( 0, 4 ) == "true" )
		{
			m_pos += 4;
			return true;
		}

		switch ( truefalse.front() )
		{
			case '0': m_pos++;  return false;
			case '1': m_pos++;  return true;
			default: throw MetaIOException( stdx::format( "Error reading boolean at pos {}", m_pos ) );
		}
	}

	std::string_view readString() override
	{
		skipWhitespace();
		if ( m_data[ m_pos ] != '"' )
			throw MetaIOException( stdx::format( "Error reading string at pos {}", m_pos ) );
		++m_pos;
		char* dest = m_data.data() + m_pos;
		const char* src = dest;
		const char* const start = dest;

		bool escaped = false;
		while ( *src != '"' || escaped )
		{
			if ( escaped )
			{
				switch ( *src )
				{
					case 'b': *dest = '\b'; break;
					case 'f': *dest = '\f'; break;
					case'n': *dest = '\n'; break;
					case 'r': *dest = '\r'; break;
					case 't': *dest = '\t'; break;
					case '"': *dest = '"'; break;
					case '\\': *dest = '\\'; break;
					default:
						throw MetaIOException( stdx::format( "Unknown escape character \"\\{}\"", *src ) );
				}
				escaped = false;
				++dest;
				++src;
			}
			else if ( *src == '\\' )
			{
				escaped = true;
				++src;
			}
			else
			{
				*dest = *src;
				++dest;
				++src;
			}
		}
		m_pos += std::distance( start, src ) + 1;
		return std::string_view( start, narrow_cast<size_t>( dest - start ) );
	}

	void startArray() override
	{
		skipWhitespace();
		if ( m_data[ m_pos ] != '[' )
			throw MetaIOException( stdx::format( "Expected array at pos {}", m_pos ) );

		m_pos++;
		skipWhitespace();
	}

	bool hasNextArrayElement( size_t count ) override
	{
		skipWhitespace();
		if ( m_data[ m_pos ] == ']' )
		{
			m_pos++;
			return false;
		}

		if ( count == 0 )
			return true;

		if ( m_data[ m_pos ] == ',' )
		{
			m_pos++;
			return true;
		}

		throw MetaIOException( stdx::format( "Expected ']' or ',' at pos {}", m_pos ) );
	}

	void startObject() override
	{
		skipWhitespace();
		if ( m_data[ m_pos ] != '{' )
			throw MetaIOException( stdx::format( "Expected object at pos {}", m_pos ) );

		m_pos++;
	}

	bool hasNextObjectVariable( size_t count ) override
	{
		skipWhitespace();
		if ( m_data[ m_pos ] == '}' )
		{
			m_pos++;
			return false;
		}

		if ( count == 0 )
			return true;

		if ( m_data[ m_pos ] == ',' )
		{
			m_pos++;
			return true;
		}

		throw MetaIOException( stdx::format( "Expected '}' or ',' at pos {}", m_pos ) );
	}

	std::string_view startVariable() override
	{
		skipWhitespace();
		auto name = readString();
		if ( m_data[ m_pos ] != ':' )
			throw MetaIOException( stdx::format( "Expected ':' at pos {}", m_pos ) );

		m_pos++;
		return name;
	}

	void endVariable() override {}

	bool isNull() override
	{
		skipWhitespace();
		const auto nullView = std::string_view( m_data ).substr( m_pos, 4 );
		const bool null = ( nullView == "null" );
		if ( null )
			m_pos += 4;

		return null;
	}

	bool eof() override
	{
		skipWhitespace();
		return m_pos == m_data.size();
	}

private:
	void skipWhitespace()
	{
		while ( std::isspace( m_data[ m_pos ] ) )
		{
			++m_pos;
		}
	}

	int readIntPrefix()
	{
		if ( m_data[ m_pos ] == '0' )
		{
			// expect octal or hexadecimal
			m_pos++;
			if ( m_data[ m_pos ] == 'x' || m_data[ m_pos ] == 'X' )
			{
				m_pos++;
				return 16;
			}
			else
			{
				return 8;
			}
		}
		else if ( m_data[ m_pos ] == '$' )
		{
			m_pos++;
			return 16;
		}

		return 10;
	}

private:
	std::string m_data;
	size_t m_pos = 0;
};

} // namespace Meta