#pragma once

#ifndef SHIPPING

#include <stdx/assert.h>
#include <stdx/flat_map.h>

#include <chrono>
#include <ostream>
#include <string_view>
#include <vector>

namespace Profiler
{

class Profile
{
public:
	Profile( std::string_view name, std::string_view filename, size_t line )
		: m_name{ name }
		, m_filename{ filename }
		, m_line{ line }
	{
		dbAssert( std::find( s_profiles.begin(), s_profiles.end(), this ) == s_profiles.end() );
		s_profiles.push_back( this );
	}

	Profile( const Profile& ) = delete;
	Profile& operator=( const Profile& ) = delete;

	void IncCall() { m_calls++; }

	void AddParentCall( std::string_view parentName )
	{
		m_parentCalls[ parentName ]++;
	}

	void AddChildCall( std::string_view childName, double time )
	{
		auto& data = m_childCalls[ childName ];
		data.calls++;
		data.totalTime += time;
	}

	void AddTime( double time ) { m_totalTime += time; }

	std::string_view GetName() const { return m_name; }

	double GetTotalTime() const { return m_totalTime; }

	void Log( std::ostream& out );

	static const auto& GetProfiles() { return s_profiles; }

private:
	struct ChildData
	{
		size_t calls = 0;
		double totalTime = 0.0;
	};

private:
	std::string_view m_name;
	std::string_view m_filename;
	size_t m_line;

	size_t m_calls = 0;
	double m_totalTime = 0.0;

	stdx::flat_map<std::string_view, size_t> m_parentCalls;
	stdx::flat_map<std::string_view, ChildData> m_childCalls;

	static std::vector<Profile*> s_profiles;
};

class ProfileBlock
{
public:
	ProfileBlock( Profile& profile );
	~ProfileBlock();

	ProfileBlock( const ProfileBlock& ) = delete;
	ProfileBlock& operator=( const ProfileBlock& ) = delete;

private:
	Profile* m_profile;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};

} // namespace Profiler

#define PROFILE_BLOCK \
	static Profiler::Profile scopedProfileData{ __FUNCSIG__, __FILE__, static_cast<size_t>( __LINE__ ) }; \
	Profiler::ProfileBlock scopedProfileBlock( scopedProfileData );

#else

#define PROFILE_BLOCK

#endif