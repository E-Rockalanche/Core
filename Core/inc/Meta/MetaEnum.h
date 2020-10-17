#pragma once

#include "MetaType.h"
#include "MetaIO.h"

#include <stdx/assert.h>
#include <stdx/enum.h>

namespace Meta
{

namespace detail
{

	template <typename E>
	E metaReadEnumValue( MetaReader& reader )
	{
		auto name = reader.readString();
		auto result = stdx::enum_cast<E>( name );
		if ( !result )
			throw MetaIOException( stdx::format( "Value \"{}\" does not exist in enum \"{}\"", name, stdx::reflection::type_name_v<E> ) );

		return *result;
	}

}

template <typename E>
class MetaEnum : public MetaType
{
	static_assert( std::is_enum_v<E> );

public:
	MetaEnum( std::string name ) : MetaType( std::move( name ) ) {}

	MetaEnum() : MetaType( stdx::reflection::type_name_v<E> ) {}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
	TypeTag getType() const override { return TypeTag::Enum; }
	void save( ByteWriter& out ) const override;
	void load( ByteReader& in ) override;
};

template <typename E>
void MetaEnum<E>::write( MetaWriter& writer, const void* data ) const
{
	E value = *static_cast<const E*>( data );
	if constexpr ( stdx::is_bitset_enum_v<E> )
	{
#ifdef DEBUG
		E check = static_cast<E>( 0 );
#endif

		writer.startArray();
		for ( auto[ bit, name ] : stdx::enum_pairs_v<E> )
		{
			if ( stdx::any_of( value, bit ) )
			{
				writer.writeString( name );
				writer.delimitArray();

#ifdef DEBUG
				check |= bit;
#endif
			}
		}
		writer.endArray();

#ifdef DEBUG
		dbAssertMessage( check == value, "bits of enum bitset cannot be saved" );
#endif
	}
	else
	{
		writer.writeString( stdx::enum_name( value ) );
	}
}

template <typename E>
void MetaEnum<E>::read( MetaReader& reader, void* data ) const
{
	E& value = *static_cast<E*>( data );
	if constexpr ( stdx::is_bitset_enum_v<E> )
	{
		E bits = static_cast<E>( 0 );

		for ( size_t count = 0; reader.hasNextArrayElement( count ); ++count )
		{
			bits |= detail::metaReadEnumValue<E>( reader );
		}
		value = bits;
	}
	else
	{
		value = detail::metaReadEnumValue<E>( reader );
	}
}

template <typename E>
void MetaEnum<E>::save( ByteWriter& out ) const
{
	MetaType::save( out );
	out.WriteHeader( static_cast<uint32_t>( TypeTag::Enum ), 0 );
	out.WriteUint32( stdx::narrow_cast<uint32_t>( stdx::enum_count_v<E> ) );
	for ( auto&[ value, name ] : stdx::enum_pairs_v<E> )
	{
		out.WriteInt64( stdx::enum_int_cast<int64_t>( value ) );
		out.WriteString( name );
	}
	out.WriteBool( stdx::is_bitset_enum_v<E> );
}

template <typename E>
void MetaEnum<E>::load( ByteReader& in )
{
	dbBreak();
}

} // namespace Meta