#pragma once

#include "Name.h"

#include <stdx/assert.h>
#include <stdx/int.h>
#include <stdx/utility.h>

#include <mutex>
#include <unordered_map>

struct StringIntern
{
public:
	using size_type = typename Name::size_type;

	static const StringIntern* Create( std::string_view v )
	{
		auto buffer = new char[ sizeof( StringIntern ) + v.size() + 1 ];

		char* strStart = buffer + sizeof( StringIntern );
		std::copy( v.begin(), v.end(), strStart );
		strStart[ v.size() - 1 ] = '\0';

		new ( buffer ) StringIntern{ stdx::narrow_cast<size_type>( v.size() ) };

		return reinterpret_cast<const StringIntern*>( buffer );
	}

	size_type size() const noexcept { return m_size; }

	const char* c_str() const noexcept { return reinterpret_cast<const char*>( this ) + sizeof( StringIntern ); }

	const char* data() const noexcept { return c_str(); }

	operator std::string_view() const noexcept { return { c_str(), m_size }; }

private:
	StringIntern( size_type s ) : m_size{ s } {}

	size_type m_size;
};

namespace
{
std::mutex s_nameMapMutex;

using name_hash_t = uint32_t;

std::unordered_map<name_hash_t, const StringIntern*> s_nameMap;
}

const StringIntern* Name::EmptyString = StringIntern::Create( "" );

Name::Name( const char* str )
{
	if ( str[ 0 ] == '\0' )
	{
		m_str = EmptyString;
	}
	else
	{
		std::string_view view{ str };
		const auto hash = stdx::hash_fnv1a<name_hash_t>( view );

		std::lock_guard lock( s_nameMapMutex );

		auto it = s_nameMap.find( hash );
		if ( it == s_nameMap.end() )
		{
			m_str = StringIntern::Create( view );
			s_nameMap.insert( { hash, m_str } );
		}
		else
		{
			dbAssert( *( it->second ) == view );
			m_str = it->second;
		}
	}
}

Name::operator view_type() const noexcept
{
	return *m_str;
}

Name::operator bool() const noexcept
{
	return m_str->size() != 0;
}

const char* Name::c_str() const noexcept
{
	return m_str->c_str();
}

const char* Name::data() const noexcept
{
	return m_str->data();
}

typename Name::size_type Name::size() const noexcept
{
	return m_str->size();
}