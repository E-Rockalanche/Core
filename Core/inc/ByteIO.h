#pragma once

#include <stdx/enum.h>
#include <stdx/type_traits.h>

#include <stdexcept>
#include <iostream>
#include <fstream>

class ByteWriter
{
public:
	ByteWriter( const char* filename ) : m_output{ filename, std::ios::binary | std::ios::out } {}

	bool IsOpen() const { return m_output.is_open(); }

	bool Open( const char* filename )
	{
		m_output.open( filename, std::ios::binary | std::ios::in );
		return IsOpen();
	}

	void WriteTag( uint32_t tag )
	{
		WriteUint32( tag );
	}

	void WriteHeader( uint32_t tag, uint32_t version )
	{
		WriteTag( tag );
		WriteUint32( version );
	}

	void WriteInt8( int8_t value ) { WritePrimitive( value ); }
	void WriteInt16( int16_t value ) { WritePrimitive( value ); }
	void WriteInt32( int32_t value ) { WritePrimitive( value ); }
	void WriteInt64( int64_t value ) { WritePrimitive( value ); }

	void WriteUint8( uint8_t value ) { WritePrimitive( value ); }
	void WriteUint16( uint16_t value ) { WritePrimitive( value ); }
	void WriteUint32( uint32_t value ) { WritePrimitive( value ); }
	void WriteUint64( uint64_t value ) { WritePrimitive( value ); }

	void WriteFloat( float value ) { WritePrimitive( value ); }
	void WriteDouble( double value ) { WritePrimitive( value ); }

	void WriteBool( bool value ) { WritePrimitive( value ); }

	template <typename T>
	void WriteString( const T& str );

	template <typename T>
	void WriteArray( const T* data, std::size_t arraySize );

	void WriteBytes( const void* data, std::size_t bytesize )
	{
		m_output.write( static_cast<const char*>( data ), bytesize );
	}

private:

	template <typename T>
	void WritePrimitive( T value )
	{
		static_assert( std::is_integral_v<T> || std::is_floating_point_v<T> );
		m_output.write( reinterpret_cast<const char*>( &value ), sizeof( T ) );
	}

	std::ofstream m_output;
};

template <typename T>
void ByteWriter::WriteString( const T& str )
{
	static_assert( std::is_same_v<T::value_type, char>, "can only write utf8 strings" );

	WriteUint32( stdx::narrow_cast<uint32_t>( str.size() ) );
	m_output.write( str.data(), str.size() );
}

template <typename T>
void ByteWriter::WriteArray( const T* data, std::size_t arraySize )
{
	static_assert( std::is_integral_v<T> || std::is_floating_point_v<T>, "can only write array of primitives" );

	m_output.write( static_cast<const char*>( data ), sizeof( T ) * arraySize );
}

class ByteReader
{
public:
	ByteReader( const char* filename ) : m_input{ filename, std::ios::binary | std::ios::in } {}

	bool IsOpen() const { return m_input.is_open(); }

	bool Open( const char* filename )
	{
		m_input.open( filename, std::ios::binary | std::ios::in );
		return IsOpen();
	}

	void ReadTag( uint32_t tag )
	{
		if ( ReadUint32() != tag )
			throw std::runtime_error( "ByteReader header is corrupt" );
	}

	// returns version number
	[[nodiscard]] uint32_t ReadHeader( uint32_t tag )
	{
		ReadTag( tag );
		return ReadUint32();
	}

	int8_t ReadInt8() { return ReadPrimitive<int8_t>(); }
	int16_t ReadInt16() { return ReadPrimitive<int16_t>(); }
	int32_t ReadInt32() { return ReadPrimitive<int32_t>(); }
	int64_t ReadInt64() { return ReadPrimitive<int64_t>(); }

	uint8_t ReadUint8() { return ReadPrimitive<uint8_t>(); }
	uint16_t ReadUint16() { return ReadPrimitive<uint16_t>(); }
	uint32_t ReadUint32() { return ReadPrimitive<uint32_t>(); }
	uint64_t ReadUint64() { return ReadPrimitive<uint64_t>(); }

	float ReadFloat() { return ReadPrimitive<float>(); }
	double ReadDouble() { return ReadPrimitive<double>(); }

	bool ReadBool() { return ReadPrimitive<bool>(); }

	template <typename T>
	T ReadString();

	template <typename T>
	void ReadString( T& str )
	{
		str = ReadString<T>();
	}

	template <typename T>
	void ReadArray( T* data, std::size_t arraySize );

	void ReadBytes( void* data, std::size_t bytesize )
	{
		m_input.read( static_cast<char*>( data ), bytesize );
	}

private:

	template <typename T>
	T ReadPrimitive()
	{
		static_assert( std::is_integral_v<T> || std::is_floating_point_v<T> );
		T result;
		m_input.read( reinterpret_cast<char*>( &result ), sizeof( T ) );
		return result;
	}

	std::ifstream m_input;
};

template <typename T>
T ByteReader::ReadString()
{
	static constexpr size_t MaxStringLength = 1024;
	const auto length = ReadUint32();
	dbAssert( length <= MaxStringLength );
	char cbuf[ MaxStringLength ];
	m_input.read( cbuf, length );
	return T( cbuf, length );
}

template <typename T>
void ByteReader::ReadArray( T* data, std::size_t arraySize )
{
	static_assert( std::is_integral_v<T> || std::is_floating_point_v<T>, "can only read array of primitives" );

	m_input.read( static_cast<char*>( data ), sizeof( T ) * arraySize );
}