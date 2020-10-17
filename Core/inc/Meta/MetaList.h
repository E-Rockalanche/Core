#pragma once

#include "MetaType.h"

#include <stdx/container.h>
#include <stdx/iterator.h>
#include <stdx/type_traits.h>

namespace Meta
{

template <typename List>
class MetaList : public MetaType
{
public:
	using value_type = typename stdx::container_traits<List>::value_type;

	MetaList()
		: MetaType( stdx::reflection::type_name_v<List> )
		, m_elementType{ getMetaType<value_type>() }
	{
		if constexpr ( stdx::is_array_like_v<List> )
		{
			m_hasStaticSize = true;
			m_staticSize = std::size( std::declval<List>() );
			dbAssert( m_staticSize != 0 );
		}
	}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
	TypeTag getType() const override { return TypeTag::List; }
	void save( ByteWriter& out ) const override;
	void load( ByteReader& in ) override;

	const MetaType* getElementType() const { return m_elementType; }

private:
	const MetaType* m_elementType;
	bool			m_hasStaticSize = false;
	std::size_t		m_staticSize = 0;
};

template <typename List>
void MetaList<List>::write( MetaWriter& writer, const void* data ) const
{
	const List& list = *static_cast<const List*>( data );

	writer.startArray();
	for ( auto it = std::begin( list ), last = std::end( list ); it != last; ++it )
	{
		if ( it != list.begin() )
			writer.delimitArray();

		m_elementType->write( writer, stdx::to_address( it ) );
	}
	writer.endArray();
}

template <typename List>
void MetaList<List>::read( MetaReader& reader, void* data ) const
{
	List& list = *static_cast<List*>( data );

	reader.startArray();

	if constexpr ( stdx::is_list_v<List> )
	{
		const size_t maxSize = list.max_size();
		for ( size_t count = 0; reader.hasNextArrayElement( count ); ++count )
		{
			if ( count == maxSize )
				throw MetaIOException( stdx::format( "Exceeding max size of list: {}", maxSize ) );

			auto& element = list.emplace_back();
			m_elementType->read( reader, std::addressof( element ) );
		}
	}
	else
	{
		static_assert( stdx::is_array_like_v<List> );

		dbAssert( std::size( list ) == m_staticSize );
		for ( size_t i = 0; i < m_staticSize; ++i )
		{
			if ( !reader.hasNextArrayElement( i ) )
				throw MetaIOException( stdx::format( "Only {} elements were read into array of size {}", i, m_staticSize ) );

			m_elementType->read( reader, std::addressof( list[ i ] ) );
		}

		if ( reader.hasNextArrayElement( m_staticSize ) )
			throw MetaIOException( stdx::format( "Too many elements read into array of size {}", m_staticSize ) );
	}
}

template <typename T>
void MetaList<T>::save( ByteWriter& out ) const
{
	out.WriteHeader( static_cast<uint32_t>( TypeTag::List ), 0 );
	out.WriteString( m_elementType->getName() );
	out.WriteBool( m_hasStaticSize );
	if ( m_hasStaticSize )
		out.WriteUint32( stdx::narrow_cast<uint32_t>( m_staticSize ) );
}

template <typename T>
void MetaList<T>::load( ByteReader& in )
{
	dbVerify( in.ReadHeader( static_cast<uint32_t>( TypeTag::List ) ) == 0 );
	auto elementTypeName = in.ReadString<std::string>();
	m_elementType = ResolveMetaType( elementTypeName );
	m_hasStaticSize = in.ReadBool();
	if ( m_hasStaticSize )
		m_staticSize = in.ReadUint32();
}

} // namespace Meta