#pragma once

#include "MetaType.h"
#include "MetaIO.h"

#include <stdx/format.h>

namespace Meta
{

template <typename T>
class MetaPrimitive : public MetaType
{
	static_assert( !std::is_pointer_v<T>, "Use MetaPointer" );
	static_assert( !std::is_enum_v<T>, "Use MetaEnum" );

public:
	MetaPrimitive( std::string name ) : MetaType( std::move( name ) ) {}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
	TypeTag getType() const override;
	void save( ByteWriter& ) const override {}
	void load( ByteReader& ) override {}
};

template <typename T>
void MetaPrimitive<T>::write( MetaWriter& writer, const void* data ) const
{
	const T& value = *static_cast<const T*>( data );

	if constexpr ( std::is_same_v<T, bool> )
	{
		writer.writeBool( *static_cast<const bool*>( data ) );
	}
	else if constexpr ( std::is_integral_v<T> )
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

	if constexpr ( std::is_same_v<T, bool> )
	{
		*static_cast<bool*>( data ) = reader.readBool();
	}
	else if constexpr ( std::is_integral_v<T> )
	{
		const int64_t intermediate = reader.readInt();
		T result = static_cast<T>( intermediate );

		if ( static_cast<int64_t>( result ) != intermediate || ( ( result < 0 ) != ( intermediate < 0 ) ) )
			throw MetaIOException( stdx::format( "Narrowing conversion of integer {} to type {}", intermediate, getName() ) );

		value = result;
	}
	else if constexpr ( std::is_floating_point_v<T> )
	{
		value = static_cast<T>( reader.readFloat() );
	}
	else
	{
		static_assert( "Cannot read primitive" );
		(void)value;
	}
}

template <typename T>
TypeTag MetaPrimitive<T>::getType() const
{
	if constexpr ( std::is_same_v<T, bool> )
	{
		return TypeTag::Bool;
	}
	else if constexpr ( std::is_integral_v<T> )
	{
		if constexpr ( std::is_signed_v<T> )
			return TypeTag::Integer;
		else
			return TypeTag::UInteger;
	}
	else if constexpr ( std::is_floating_point_v<T> )
	{
		return TypeTag::Real;
	}
	else
	{
		static_assert( false, "invalid primitve type" );
	}
}

} // namespace Meta