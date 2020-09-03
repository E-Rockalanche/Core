#pragma once

#include "MetaType.h"

#include <stdx/reflection.h>

namespace Meta
{

class MetaVariable
{
public:
	MetaVariable( const MetaType* type, std::string_view name, size_t offset )
		: m_type{ type }, m_name{ name }, m_offset{ offset }
	{}

	const MetaType* getType() const { return m_type; }
	std::string_view getName() const { return m_name; }
	size_t getOffset() const { return m_offset; }

private:
	const MetaType* m_type;
	std::string_view m_name;
	size_t m_offset;
};

class MetaClass : public MetaType
{
public:
	// base class
	template<typename T>
	static MetaClass MakeMetaClass( std::vector<MetaVariable> variables )
	{
		return MetaClass(
			stdx::reflection::template type_name_v<T>,
			std::is_polymorphic_v<T>,
			std::is_abstract_v<T>,
			nullptr,
			std::move( variables ) );
	}

	// derived class
	template<typename T>
	static MetaClass MakeMetaClass( const MetaClass* parent, std::vector<MetaVariable> variables )
	{
		return MetaClass(
			stdx::reflection::template type_name_v<T>,
			std::is_polymorphic_v<T>,
			std::is_abstract_v<T>,
			parent,
			std::move( variables ) );
	}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;

	const auto& getVariables() const { return m_variables; }

	const MetaVariable* findVariable( std::string_view name ) const;

	bool hasVariable( std::string_view name ) const
	{
		return findVariable( name ) != nullptr;
	}

	bool isBase() const { return m_parent == nullptr; }
	bool isPolymorphic() const { return m_polymorphic; }
	bool isAbstract() const { return m_abstract; }

private:

	MetaClass( std::string name, bool polymorphic, bool abstract, const MetaClass* parent, std::vector<MetaVariable> variables )
		: MetaType( std::move( name ) )
		, m_variables{ std::move( variables ) }
		, m_parent{ parent }
		, m_polymorphic{ polymorphic }
		, m_abstract{ abstract }
	{}

private:
	std::vector<MetaVariable> m_variables;

	const MetaClass* m_parent = nullptr;
	bool m_polymorphic;
	bool m_abstract;
};

} // namespace Meta