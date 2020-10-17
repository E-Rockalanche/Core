
#include <stdx/enum.h>
#include <stdx/format.h>
#include <stdx/iterator.h>

#include <Meta/MetaType.h>
#include <Meta/MetaClass.h>
#include <Meta/MetaIO.h>

namespace Meta
{

const MetaVariable* MetaClass::findVariable( std::string_view name ) const
{
	auto it = std::find_if( m_variables.begin(), m_variables.end(), [name]( auto& var ) { return var.name == name; } );
	if ( it != m_variables.end() )
		return stdx::to_address( it );

	if ( m_parent != nullptr )
		return m_parent->findVariable( name );

	return nullptr;
}

void MetaClass::write( MetaWriter& writer, const void* data ) const
{
	writer.startObject();

	writer.startVariable( "CLASSNAME" );
	writer.writeString( getName() );
	writer.endVariable();

	for ( auto& metavar : m_variables )
	{
		writer.delimitObject();
		writer.startVariable( metavar.name );
		metavar.type->write( writer, (const char*)data + metavar.offset );
		writer.endVariable();
	}

	writer.endObject();
}

void MetaClass::read( MetaReader& reader, void* data ) const
{
	reader.startObject();

	auto expectClassNameKey = reader.startVariable();
	if ( expectClassNameKey != "CLASSNAME" )
		throw MetaIOException( "CLASSNAME must be first variable in class object" );

	auto className = reader.readString();
	if ( className != getName() )
		throw MetaIOException( stdx::format( "Class name mismatch. Expected \"{}\". Read \"{}\"", getName(), className ) );

	reader.endVariable();

	size_t count = 1;
	while ( reader.hasNextObjectVariable( count ) )
	{
		auto varname = reader.startVariable();
		auto* metavar = findVariable( varname );
		if ( metavar == nullptr )
			throw MetaIOException( stdx::format( "Class \"{}\" has no member \"{}\"", getName(), varname ) );

		metavar->type->read( reader, (char*)data + metavar->offset );
		reader.endVariable();
		++count;
	}
}

constexpr uint32_t Tag = 'CLAS';

void MetaClass::save( ByteWriter& out ) const
{
	out.WriteHeader( Tag, 0 );

	out.WriteBool( m_parent );
	if ( m_parent )
		out.WriteString( m_parent->getName() );

	out.WriteBool( m_polymorphic );
	out.WriteBool( m_abstract );

	out.WriteUint8( stdx::narrow_cast<uint8_t>( m_variables.size() ) );
	for ( auto& var : m_variables )
	{
		out.WriteString( var.type->getName() );
		out.WriteString( var.name );
		out.WriteUint32( stdx::narrow_cast<uint32_t>( var.offset ) );
	}
}

void MetaClass::load( ByteReader& in )
{
	dbVerify( in.ReadHeader( Tag ) == 0 );

	if ( in.ReadBool() )
	{
		auto parentTypeName = in.ReadString<std::string>();
		// resolve parent type from name
	}

	m_polymorphic = in.ReadBool();
	m_abstract = in.ReadBool();

	auto varCount = in.ReadUint8();
	m_variables.reserve( varCount );
	for ( ; varCount > 0; --varCount )
	{
		MetaVariable v;

	}
}

} // namepsace Meta