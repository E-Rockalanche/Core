#pragma once

#include "MetaType.h"

#include <stdx/reflection.h>

namespace Meta
{

struct MetaVariable
{
	const MetaType* type = nullptr;
	std::string name;
	size_t offset = 0;
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

	MetaClass( std::string name ) : MetaType( std::move( name ) ) {}

	MetaClass( std::string name, bool polymorphic, bool abstract, const MetaClass* parent, std::vector<MetaVariable> variables )
		: MetaType( std::move( name ) )
		, m_variables{ std::move( variables ) }
		, m_parent{ parent }
		, m_polymorphic{ polymorphic }
		, m_abstract{ abstract }
	{}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
	TypeTag getType() const override { return TypeTag::Class; }
	void save( ByteWriter& out ) const override;
	void load( ByteReader& in ) override;

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

private:
	std::vector<MetaVariable> m_variables;

	const MetaClass* m_parent = nullptr;
	bool m_polymorphic = false;
	bool m_abstract = false;
};

} // namespace Meta