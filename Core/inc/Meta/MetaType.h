#pragma once

#include "MetaExport.h"

#include "ByteIO.h"

#include <stdx/memory.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace Meta
{

enum class TypeTag : uint32_t
{
	Bool = 'BOOL',
	Integer = 'SINT',
	UInteger = 'UINT',
	Real = 'REAL',
	Class = 'CLAS',
	Enum = 'ENUM',
	List = 'LIST',
	Map = 'MAPP',
	Pointer = 'POIN',
	Set = 'SETT',
	String = 'STRI'
};

class MetaType;
class MetaWriter;
class MetaReader;

class MetaType
{
public:
	MetaType( std::string name ) : m_name{ std::move( name ) }
	{
		RegisterMetaType( this );
	}

	MetaType( std::string_view name ) : MetaType{ std::string{ name.begin(), name.end() } } {}

	MetaType( const MetaType& ) = delete;
	MetaType( MetaType&& ) = delete;
	MetaType& operator=( const MetaType& ) = delete;
	MetaType& operator=( MetaType&& ) = delete;

	std::string_view getName() const { return m_name; }

	virtual void write( MetaWriter& writer, const void* data ) const = 0;
	virtual void read( MetaReader& reader, void* data ) const = 0;
	virtual TypeTag getType() const = 0;

	virtual void save( ByteWriter& out ) const
	{
		// read in by loader and used to select which type to create
		out.WriteUint32( static_cast<uint32_t>( getType() ) );
		out.WriteString( m_name );
	}

	virtual void load( ByteReader& ) { dbBreak(); } // loading only for meta editor

private:
	std::string m_name;
};

// specialize this class to get meta type object of containers
template <typename T>
struct MetaTypeResolver
{
	static const MetaType* get();
};

template <typename T>
const MetaType* getMetaType()
{
	return MetaTypeResolver<T>::get();
}

} // namespace Meta