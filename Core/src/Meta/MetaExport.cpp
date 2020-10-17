#include "Meta/MetaExport.h"

#include "Meta/MetaClass.h"
#include "Meta/MetaEnum.h"
#include "Meta/MetaList.h"
#include "Meta/MetaMap.h"
#include "Meta/MetaPointer.h"
#include "Meta/MetaPrimitive.h"
#include "Meta/MetaSet.h"
#include "Meta/MetaString.h"

#include "ByteIO.h"

#include <stdx/json.h>
#include <stdx/simple_map.h>

namespace Meta
{

enum class DummyEnum : int64_t {};

stdx::simple_map<std::string_view, const MetaType*> s_metaTypes;

void RegisterMetaType( const MetaType* type )
{
	auto result = s_metaTypes.insert( { type->getName(), type } );
	dbAssert( result.first->second == type );
}

const MetaType* ResolveMetaType( std::string_view typeName )
{
	for ( auto&[ name, type ] : s_metaTypes )
	{
		if ( name == typeName )
			return type;
	}

	dbBreak();
	return nullptr;
}

constexpr uint32_t BeginTag = 'MBGN';
constexpr uint32_t EndTag = 'MEND';

void ExportMetaTypes( const char* filename )
{
	ByteWriter out( filename );
	if ( !out.IsOpen() )
		throw std::runtime_error( "cannot open file for meta export" );

	out.WriteHeader( BeginTag, 0 );
	out.WriteUint32( stdx::narrow_cast<uint32_t>( s_metaTypes.size() ) );
	for ( auto&[ name, type ] : s_metaTypes )
	{
		out.WriteUint8( stdx::enum_int_cast<uint8_t>( type->getType() ) );
		type->save( out );
	}
	out.WriteTag( EndTag );
}

template <typename T>
void CreateMetaPrimitive( std::string name )
{
	if ( !ResolveMetaType( name ) )
		new MetaPrimitive<T>( name );
}

void LoadMetaTypes( const char* filename )
{
	ByteReader in( filename );
	if ( !in.IsOpen() )
		throw std::runtime_error( "cannot open file for meta load" );

	dbVerify( in.ReadHeader( BeginTag ) == 0 );
	auto typeCount = in.ReadUint32();
	for ( ; typeCount > 0; --typeCount )
	{
		const auto typeIndex = *stdx::enum_cast<TypeTag>( in.ReadUint8() );
		auto name = in.ReadString<std::string>();
		switch ( typeIndex )
		{
			case TypeTag::Bool:
				CreateMetaPrimitive<bool>( std::move( name ) );
				break;

			case TypeTag::Integer:
				CreateMetaPrimitive<int64_t>( std::move( name ) );
				break;

			case TypeTag::UInteger:
				CreateMetaPrimitive<uint64_t>( std::move( name ) );
				break;

			case TypeTag::Real:
				CreateMetaPrimitive<double>( std::move( name ) );
				break;

			case TypeTag::Class:
			{
				auto type = new MetaClass( std::move( name ) );
				type->load( in );
				break;
			}

			case TypeTag::Enum:
			{
				auto type = new MetaEnum<DummyEnum>( std::move( name ) );
				type->load( in );
				break;
			}

			case TypeTag::List:
			{
				auto type = new MetaList<std::vector<stdx::json>>();
			}
		}
	}
}

}