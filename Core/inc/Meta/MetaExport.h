#pragma once

#include <string_view>

namespace Meta
{

class MetaType;

void RegisterMetaType( const MetaType* type );
const MetaType* ResolveMetaType( std::string_view typeName );

void ExportMetaTypes( const char* filename );
void LoadMetaTypes( const char* filename );

}