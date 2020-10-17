#pragma once

#include <stdx/int.h>

#include <exception>
#include <optional>
#include <string>
#include <string_view>

namespace Meta
{

class MetaType;

class MetaWriter
{
public:
	template <typename T>
	bool write( const T& value )
	{
		const MetaType* metatype = getMetaType<T>();
		try
		{
			metatype->write( *this, std::addressof( value ) );
			return true;
		}
		catch ( const std::exception& e )
		{
			dbLogError( "MetaWriter caught exception: %s", e.what() );
			return false;
		}
	}

	virtual void writeInt( int64_t value, int base = 10 ) = 0;
	virtual void writeFloat( double value ) = 0;
	virtual void writeBool( bool value ) = 0;
	virtual void writeString( std::string_view str ) = 0;

	virtual void startArray() = 0;
	virtual void delimitArray() = 0;
	virtual void endArray() = 0;

	virtual void startObject() = 0;
	virtual void delimitObject() = 0;
	virtual void endObject() = 0;

	virtual void startVariable( std::string_view name ) = 0;
	virtual void endVariable() = 0;

	virtual void writeNull() = 0;
};

class MetaReader
{
public:
	template <typename T>
	bool read( T& value )
	{
		const MetaType* metatype = getMetaType<T>();
		try
		{
			metatype->read( *this, std::addressof( value ) );
			return true;
		}
		catch ( const std::exception& e )
		{
			dbLogError( "MetaReader caught exception: %s", e.what() );
			return false;
		}
	}

	virtual int64_t readInt() = 0;
	virtual double readFloat() = 0;
	virtual bool readBool() = 0;
	virtual std::string_view readString() = 0;

	virtual void startArray() = 0;
	virtual bool hasNextArrayElement( size_t count ) = 0;

	virtual void startObject() = 0;
	virtual bool hasNextObjectVariable( size_t count ) = 0;

	virtual std::string_view startVariable() = 0;
	virtual void endVariable() = 0;

	virtual bool isNull() = 0;

	virtual bool eof() = 0;
};

class MetaIOException : public std::exception
{
public:
	MetaIOException( std::string_view message ) : m_message( message ) {}

	const char* what() const noexcept override { return m_message.c_str(); }

private:
	std::string m_message;
};

} // namespace Meta