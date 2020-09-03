#pragma once

#include "MetaType.h"

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
			throw MetaIOException( stdx::format( "Value \"{}\" does not exist in enum \"{}\"", name, getName() ) );

		return *result;
	}

}

template <typename E>
class MetaEnum : public MetaType
{
	static_assert( std::is_enum_v<E> );

public:
	MetaEnum() : MetaType( stdx::reflection::type_name_v<E> ) {}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
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

} // namespace Meta