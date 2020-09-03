#include "Profiler.h"

#include <stdx/assert.h>

#include <iostream>
#include <fstream>

namespace Profiler
{

std::vector<Profile*> Profile::s_profiles;

void Profile::Log( std::ostream& out )
{
	std::vector<std::pair<std::string_view, size_t>> sortedParentCalls{ m_parentCalls.begin(), m_parentCalls.end() };
	std::sort( sortedParentCalls.begin(), sortedParentCalls.end(), []( auto& lhs, auto& rhs ) { return lhs.second > rhs.second; } );

	std::vector<std::pair<std::string_view, ChildData>> sortedChildCalls{ m_childCalls.begin(), m_childCalls.end() };
	std::sort( sortedChildCalls.begin(), sortedChildCalls.end(), []( auto& lhs, auto& rhs ) { return lhs.second.totalTime > rhs.second.totalTime; } );

	constexpr double NanoSecondsPerSecond = 1000000000.0;

	dbAssert( m_totalTime > 0 );

	out << m_name
		<< "\n\tlocation:   " << m_filename << '[' << m_line << ']'
		<< "\n\tcalls:	    " << m_calls
		<< "\n\ttotal time: " << m_totalTime
		<< '\n';

	if ( !sortedParentCalls.empty() )
	{
		out << "\tcalled from:";
		for ( auto[ parentName, calls ] : sortedParentCalls )
		{
			out << "\t\t" << parentName << " (" << calls << ")\n";
		}
	}

	if ( !sortedChildCalls.empty() )
	{
		out << "\tcalled:";
		for ( auto[ childName, data ] : sortedChildCalls )
		{
			auto[ calls, childTime ] = data;
			const auto percentTime = 100 * childTime / m_totalTime;
			out << "\t\t" << percentTime << "%: " << childName << " (" << ( childTime / NanoSecondsPerSecond ) << "s, " << calls << ")\n";
		}
	}

	out << '\n';
}

std::vector<Profile*> s_callstack;

class ProfileLogger
{
public:
	~ProfileLogger()
	{
		dbAssert( s_callstack.empty() );

		std::ofstream fout( "profile.log" );

		auto sortedProfiles = Profile::GetProfiles();
		std::sort( sortedProfiles.begin(), sortedProfiles.end(), []( auto* lhs, auto* rhs ) { return lhs->GetTotalTime() > rhs->GetTotalTime(); } );

		for ( auto* profile : sortedProfiles )
		{
			profile->Log( fout );
		}

		fout.close();
	}
};

ProfileLogger s_logger;

ProfileBlock::ProfileBlock( Profile& profile ) : m_profile{ &profile }, m_start{ std::chrono::high_resolution_clock::now() }
{
	m_profile->IncCall();

	if ( !s_callstack.empty() )
	{
		auto* parent = s_callstack.back();
		m_profile->AddParentCall( parent->GetName() );
	}

	s_callstack.push_back( m_profile );
}

ProfileBlock::~ProfileBlock()
{
	dbAssert( m_profile );
	dbAssert( !s_callstack.empty() );
	dbAssert( s_callstack.back() == m_profile );

	s_callstack.pop_back();

	const auto end = std::chrono::high_resolution_clock::now();
	const double elapsed = static_cast<double>( std::chrono::duration_cast<std::chrono::nanoseconds>( end - m_start ).count() );

	m_profile->AddTime( elapsed );
	if ( !s_callstack.empty() )
		s_callstack.back()->AddChildCall( m_profile->GetName(), elapsed );
}

} // namespace Profiler