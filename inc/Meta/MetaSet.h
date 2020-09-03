#pragma once

#include "MetaType.h"

#include <stdx/container.h>

namespace Meta
{

template <typename Set>
class MetaSet : public MetaType
{
public:
	using value_type = typename stdx::container_traits<Set>::value_type;

	MetaSet()
		: MetaType( stdx::format( "set<{}>", getMetaType<element_type>()->getName() ) )
		, m_elementType{ getMetaType<element_type>() }
	{}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;

	const MetaType* getElementType() const { return m_elementType; }

private:
	const MetaType* m_elementType;
};

template <typename Set>
void MetaSet<Set>::write( MetaWriter& writer, const void* data ) const
{
	const Set& list = *static_cast<const Set*>( data );

	writer.startArray();
	for ( auto it = list.begin(), last = list.end(); it != last; ++it )
	{
		if ( it != list.begin() )
			writer.delimitArray();

		m_elementType->write( writer, stdx::to_address( it ) );
	}
	writer.endArray();
}

template <typename Set>
void MetaSet<Set>::read( MetaReader& reader, void* data ) const
{
	Set& set = *static_cast<Set*>( data );

	reader.startArray();

	const size_t maxSize = set.max_size();
	for ( size_t count = 0; reader.hasNextArrayElement( count ); ++count )
	{
		if ( count >= maxSize )
			throw MetaIOException( stdx::format( "Exceeding max size of set: {}", maxSize ) );

		typename Set::value_type value{};
		m_elementType->read( reader, std::addressof( value ) );

		auto result = set.insert( std::move( value ) );
		if ( !result.second )
			throw MetaIOException( stdx::format( "Duplicate set value at element {}", count ) );
	}
}

} // namespace Meta