#pragma once

#include "MetaType.h"
#include "MetaIO.h"
#include "MetaClass.h"

#include <memory>

namespace Meta
{

template <typename Ptr>
class MetaUniquePointer : public MetaType
{
public:
	using element_type = typename std::pointer_traits<Ptr>::element_type;

	MetaUniquePointer()
		: MetaType( stdx::reflection::type_name_v<Ptr> )
		, m_type{ getMetaType<element_type>() }
	{}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
	TypeTag getType() const override { return TypeTag::Pointer; }
	void save( ByteWriter& out ) const override;
	void load( ByteReader& in ) override;

	const MetaType* getType() const { return m_type; }

private:
	const MetaType* m_type;
};

template <typename Ptr>
void MetaUniquePointer<Ptr>::write( MetaWriter& writer, const void* data ) const
{
	const auto& ptr = *static_cast<const Ptr*>( data );

	if ( ptr == nullptr )
		writer.writeNull();
	else
		m_type->write( writer, ptr.get() );
}

template <typename Ptr>
void MetaUniquePointer<Ptr>::read( MetaReader& reader, void* data ) const
{
	auto& ptr = *static_cast<Ptr*>( data );

	if ( reader.isNull() )
	{
		ptr = nullptr;
	}
	else
	{
		auto p = new element_type();
		try
		{
			m_type->read( reader, p );
		}
		catch ( MetaIOException& e )
		{
			delete p;
			throw e;
		}
		ptr = Ptr( p );
	}
}

template <typename Ptr>
void MetaUniquePointer<Ptr>::save( ByteWriter& out ) const
{
	out.WriteHeader( static_cast<uint32_t>( TypeTag::Pointer ), 0 );
	out.WriteString( m_type->getName() );
}

template <typename Ptr>
void MetaUniquePointer<Ptr>::load( ByteReader& in )
{
	dbVerify( in.ReadHeader( static_cast<uint32_t>( TypeTag::Pointer ) ) == 0 );
	auto typeName = in.ReadString<std::string>();
	m_type = ResolveMetaType( typeName );
}

} // namespace Meta