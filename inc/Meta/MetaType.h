#pragma once

#include <stdx/memory.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

class MetaWriter;
class MetaReader;

class MetaType
{
public:
	MetaType( std::string name ) : m_name{ std::move( name ) } {}

	std::string_view getName() const { return m_name; }

	virtual void write( MetaWriter& writer, const void* data ) const = 0;
	virtual void read( MetaReader& reader, void* data ) const = 0;

private:
	std::string m_name;
};

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