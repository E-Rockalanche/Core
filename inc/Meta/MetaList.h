#pragma once

#include "MetaType.h"

#include <stdx/container.h>
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
	{}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;

	const MetaType* getElementType() const { return m_elementType; }

private:
	const MetaType* m_elementType;
};

template <typename List>
void MetaList<List>::write( MetaWriter& writer, const void* data ) const
{
	const List& list = *static_cast<const List*>( data );

	writer.startArray();
	for ( auto it = list.begin(), last = list.end(); it != last; ++it )
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

	if constexpr ( stdx::is_list_v<T> )
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
		static_assert( stdx::is_array_like_v<T> );

		constexpr size_t size = std::size( list );

		for ( size_t i = 0; i < size; ++i )
		{
			if ( !reader.hasNextArrayElement( i ) )
				throw MetaIOException( stdx::format( "Only {} elements were read into array of size {}", i, size ) );

			m_elementType->read( reader, std::addressof( list[ i ] ) );
		}

		if ( reader.hasNextArrayElement( size ) )
			throw MetaIOException( stdx::format( "Too many elements read into array of size {}", size ) );
	}
}

} // namespace Meta