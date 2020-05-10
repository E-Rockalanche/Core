#pragma once

#include "MetaType.h"

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
	// for non-polymorphic or base classes and structs
	MetaClass( std::string name, bool polymorphic, bool abstract, std::vector<MetaVariable> variables )
		: MetaType( std::move( name ) )
		, m_variables{ std::move( variables ) }
		, m_polymorphic{ polymorphic }
		, m_abstract{ abstract }
	{}

	// for derrived classes and structs
	MetaClass( std::string name, bool polymorphic, bool abstract, const MetaClass* parent, std::vector<MetaVariable> variables )
		: MetaType( std::move( name ) )
		, m_variables{ std::move( variables ) }
		, m_parent{ parent }
		, m_polymorphic{ polymorphic }
		, m_abstract{ abstract }
	{
		m_parent->addChildClass( this );
	}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;

	const auto& getVariables() const { return m_variables; }

	const MetaVariable* findVariable( std::string_view name ) const;

	bool hasVariable( std::string_view name ) const
	{
		return findVariable( name ) != nullptr;
	}

	bool isDerrived() const { return m_parent != nullptr; }
	bool isBasePolymorphic() const { return m_parent == nullptr && m_polymorphic; }
	bool isMonomorphic() const { return m_parent == nullptr && !m_polymorphic; }
	bool isAbstract() const { return m_abstract; }

private:

	void addChildClass( const MetaClass* child ) const
	{
		m_children.push_back( child );
	}

private:
	std::vector<MetaVariable> m_variables;

	const MetaClass* m_parent = nullptr;
	bool m_polymorphic;
	bool m_abstract;

	mutable std::vector<const MetaClass*> m_children; // children are registered after class is created
};