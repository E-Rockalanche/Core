#pragma once

#include "MetaType.h"

#include <stdx/container.h>

namespace Meta
{

template <typename Map>
class MetaMap : public MetaType
{
public:
	using key_type = typename Map::key_type;
	using mapped_type = typename Map::mapped_type;

	MetaMap()
		: MetaType( stdx::format("map<{},{}>", getMetaType<key_type>()->getName(), getMetaType<mapped_type>()->getName() ) )
		, m_keyType{ getMetaType<key_type>() }
		, m_valueType{ getMetaType<mapped_type>() }
	{}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
	TypeTag getType() const override { return TypeTag::Map; }
	void save( ByteWriter& out ) const override;
	void load( ByteReader& in ) override;

	const MetaType* getKeyType() const { return m_keyType; }
	const MetaType* getValueType() const { return m_valueType; }

private:
	const MetaType* m_keyType;
	const MetaType* m_valueType;
};

template <typename Map>
void MetaMap<Map>::write( MetaWriter& writer, const void* data ) const
{
	const Map& map = *static_cast<const Map*>( data );

	writer.startArray();

	for ( auto it = map.begin(), last = map.end(); it != last; ++it )
	{
		if ( it != map.begin() )
			writer.delimitArray();

		auto[ key, value ] = *it;

		writer.startObject();

		writer.startVariable( "K" );
		m_keyType->write( writer, std::addressof( key ) );
		writer.endVariable();

		writer.delimitObject();

		writer.startVariable( "V" );
		m_valueType->write( writer, std::addressof( value ) );
		writer.endVariable();

		writer.endObject();
	}

	writer.endArray();
}

template <typename Map>
void MetaMap<Map>::read( MetaReader& reader, void* data ) const
{
	Map& map = *static_cast<Map*>( data );

	reader.startArray();

	for ( size_t count = 0; reader.hasNextArrayElement( count ); ++count )
	{
		reader.startObject();
		auto expectedKeyKey = reader.startVariable();
		if ( expectedKeyKey != "K" )
			throw MetaIOException( stdx::format( "Expected variable K at map element {}", count ) );

		key_type key{};
		m_keyType->read( reader, std::addressof( key ) );
		reader.endVariable();

		if ( !reader.hasNextObjectVariable( 1 ) )
			throw MetaIOException( stdx::format( "Missing value in map element {}", count ) );

		auto expectedValueKey = reader.startVariable();
		if ( expectedValueKey != "V" )
			throw MetaIOException( stdx::format( "Expected variable V at map element {}", count ) );

		mapped_type value{};
		m_valueType->read( reader, std::addressof( value ) );
		reader.endVariable();

		if constexpr ( stdx::is_detected_v<detail::insert_value_t, Map> )
		{
			auto result = map.insert( { std::move( key ), std::move( value ) } );
			if ( !result.second )
				throw MetaIOException( stdx::format( "Key {} duplicate found at element {}", key, count ) );
		}
		else
		{
			// less safe but supports stdx::enum_map
			map[ std::move( key ) ] = std::move( value );
		}

		if ( reader.hasNextObjectVariable( 2 ) )
			throw MetaIOException( "Expected end of object in map element" );
	}
}

template <typename Map>
void MetaMap<Map>::save( ByteWriter& out ) const
{
	out.WriteHeader( static_cast<uint32_t>( TypeTag::Map ), 0 );
	out.WriteString( m_keyType->getName() );
	out.WriteString( m_valueType->getName() );
}

template <typename Map>
void MetaMap<Map>::load( ByteReader& in )
{
	dbVerify( in.ReadHeader( static_cast<uint32_t>( TypeTag::Map ) == 0 ) );

	auto keyTypeName = in.ReadString<std::string>();
	m_keyType = ResolveMetaType( keyTypeName );

	auto valueTypeName = in.ReadString<std::string>();
	m_valueType = ResolveMetaType( valueTypeName );
}

} // namespace Meta