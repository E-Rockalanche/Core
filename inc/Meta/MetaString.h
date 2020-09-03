#pragma once

#include "MetaType.h"
#include "MetaIO.h"

#include <string_view>
#include <type_traits>

namespace Meta
{

template <typename Str>
class MetaString : public MetaType
{
	static_assert( std::is_convertible_v<Str, std::string_view> );

public:
	MetaString() : MetaType( "string" ) {}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
};

template <typename Str>
void MetaString<Str>::write( MetaWriter& writer, const void* data ) const
{
	writer.writeString( std::string_view{ *static_cast<const Str*>( data ) } );
}

template <typename Str>
void MetaString<Str>::read( MetaReader& reader, void* data ) const
{
	auto& value = *static_cast<Str*>( data );

	auto str = reader.readString();

	if constexpr ( std::is_convertible_v<std::string_view, Str> )
	{
		value = str;
	}
	else
	{
		constexpr auto capacity = std::size( value );
		if ( str.size() + 1 < capacity ) // +1 for null terminator
			throw MetaIOException( stdx::format( "cannot assign string {} value {}", getName(), str ) );

		auto end = std::copy( str.begin(), str.end(), std::data( value ) );
		*end = '\0';
	}
}

} // namespace Meta