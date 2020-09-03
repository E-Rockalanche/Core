
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
	auto it = std::find_if( m_variables.begin(), m_variables.end(), [name]( auto& var ) { return var.getName() == name; } );
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
		writer.startVariable( metavar.getName() );
		metavar.getType()->write( writer, (const char*)data + metavar.getOffset() );
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

		metavar->getType()->read( reader, (char*)data + metavar->getOffset() );
		reader.endVariable();
		++count;
	}
}

} // namepsace Meta