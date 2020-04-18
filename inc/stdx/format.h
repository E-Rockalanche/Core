#ifndef STDX_FORMAT_HPP
#define STDX_FORMAT_HPP

#include <stdexcept>
#include <sstream>
#include <string>
#include <string_view>

namespace stdx
{

class format_error : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

namespace detail
{
	inline size_t findFirstArg( std::string_view fmt )
	{
		size_t pos = 0;
		while ( true )
		{
			pos = fmt.find_first_of( '{', pos );
			if ( pos == std::string_view::npos )
				return pos;

			if ( pos + 1 < fmt.size() && fmt[ pos + 1 ] != '{' )
				return pos;

			pos += 2;
		}
	}

	inline void format_internal( std::stringstream& ss, std::string_view fmt )
	{
		size_t pos = findFirstArg( fmt );

		if ( pos != std::string_view::npos )
			throw format_error( "too many format specifiers" );

		ss << fmt;
	}

	template <class Head, class...Tail>
	void format_internal( std::stringstream& ss, std::string_view fmt, const Head& head, const Tail&... tail )
	{
		size_t pos = findFirstArg( fmt );

		if ( pos == std::string_view::npos )
			throw format_error( "too many arguments" );

		ss << fmt.substr( 0, pos ) << head;

		// assume empty format specifier ( "{}" )
		format_internal( ss, fmt.substr( pos + 2 ), tail... );
	}
}

template <class... Args>
std::string format( std::string_view fmt, const Args... args )
{
	std::stringstream ss;
	detail::format_internal( ss, fmt, args... );
	return ss.str();
}

}

#endif