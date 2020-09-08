#pragma once

#include <stdx/memory.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace Meta
{

class MetaType;
class MetaWriter;
class MetaReader;

void RegisterMetaType( const MetaType* type );
void ExportMetaTypes( const char* filename );

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