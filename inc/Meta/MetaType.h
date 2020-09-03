#pragma once

#include <stdx/memory.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace Meta
{

class MetaWriter;
class MetaReader;

class MetaType
{
public:
	MetaType( std::string name ) : m_name{ std::move( name ) } {}
	MetaType( std::string_view name ) : m_name{ name.begin(), name.end() } {}

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