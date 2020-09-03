#pragma once

#include "MetaType.h"
#include "MetaIO.h"

#include <stdx/format.h>

namespace Meta
{

template <typename T>
class MetaPrimitive : public MetaType<T>
{
	static_assert( !std::is_pointer_v<T>, "Use MetaPointer" );
	static_assert( !std::is_enum_v<T>, "Use MetaEnum" );

public:
	MetaPrimitive() : MetaType( stdx::reflection::type_name_v<T> ) {}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
};

template <typename T>
void MetaPrimitive<T>::write( MetaWriter& writer, const void* data ) const
{
	const T& value = *static_cast<const T*>( data );

	if constexpr ( std::is_integral_v<T> )
	{
		writer.writeInt( stdx::narrow_cast<int64_t>( value ) );
	}
	else if constexpr ( std::is_floating_point_v<T> )
	{
		writer.writeFloat( static_cast<double>( value ) );
	}
	else
	{
		static_assert( "Cannot write primitive" );
	}
}

template <typename T>
void MetaPrimitive<T>::read( MetaReader& reader, void* data ) const
{
	T& value = *static_cast<T*>( data );

	if constexpr ( std::is_integral_v<T> )
	{
		const int64_t intermediate = reader.readInt();
		T result = static_cast<T>( intermediate );

		if ( static_cast<int64_t>( result ) != intermediate || ( ( result < 0 ) != ( intermediate < 0 ) ) )
			throw MetaIOException( stdx::format( "Narrowing conversion of integer {} to type {}", intermediate, getName() ) );

		value = result;
	}
	else if constexpr ( std::is_floating_point_v<T> )
	{
		double result = reader.readFloat();
		if ( result > static_cast<double>( std::numeric_limits<T>::max() ) || result < static_cast<double>( std::numeric_limits<T>::lowest() ) )
			throw MetaIOException( stdx::format( "Narrowing conversion of real {} to type {}", result, getName() ) );

		value = static_cast<T>( result );
	}
	else
	{
		static_assert( "Cannot read primitive" );
		(void)value;
	}
}

template <>
void MetaPrimitive<bool>::write( MetaWriter& writer, const void* data ) const
{
	writer.writeBool( *static_cast<const bool*>( data ) );
}

template <>
void MetaPrimitive<bool>::read( MetaReader& reader, void* data ) const
{
	*static_cast<bool*>( data ) = reader.readBool();
}

template <>
void MetaPrimitive<char>::write( MetaWriter& writer, const void* data ) const
{
	writer.writeString( std::string_view( static_cast<const char*>( data ), 1 ) );
}

template <>
void MetaPrimitive<char>::read( MetaReader& reader, void* data ) const
{
	auto str = reader.readString();
	if ( str.size() != 1 )
		throw MetaIOException( stdx::format( "Invalid char: {}", str ) );

	*static_cast<char*>( data ) = str.front();
}

} // namespace Meta