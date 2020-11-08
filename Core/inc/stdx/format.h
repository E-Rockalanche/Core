#ifndef STDX_FORMAT_HPP
#define STDX_FORMAT_HPP

#include <stdx/assert.h>

#include <algorithm>
#include <iterator>
#include <istream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace stdx
{

class format_exception : public std::exception
{
	using std::exception::exception;
};

namespace detail
{
	// returns indices to '{' and position after '}'
	template <typename CharT, typename Traits>
	std::pair<std::size_t, std::size_t> find_format_specifier( std::basic_string_view<CharT, Traits> str )
	{
		constexpr auto npos = str.npos;

		size_t searchStart = 0;
		size_t specifierStart = 0;
		for(;;)
		{
			specifierStart = str.find_first_of( '{', searchStart );
			if ( specifierStart == npos )
				return { npos, npos };

			const auto next = specifierStart + 1;
			if ( next < str.size() && str[ next ] != '{' )
				break;

			searchStart = next + 1;
		}

		const auto specifierEnd = str.find_first_of( '}', specifierStart + 1 );
		if ( specifierEnd == npos )
			return { npos, npos };
		else
			return { specifierStart, specifierEnd + 1 };
	}

	template <typename Iterator>
	using it_diff_t = typename std::iterator_traits<Iterator>::difference_type;

	template <typename OutputIt, typename CharT>
	inline std::tuple<OutputIt, size_t, size_t> format_copy_n_return_argpos(
		const OutputIt out,
		it_diff_t<OutputIt> n,
		std::basic_string_view<CharT> fmt )
	{
		size_t copyStart = 0;
		auto newOut = out;
		while ( true )
		{
			const size_t argStart = fmt.find_first_of( '{', copyStart );

			const size_t copyEnd = std::min( { fmt.size(), argStart, n } );
			if ( copyStart < copyEnd )
				newOut = std::copy( fmt.begin() + copyStart, fmt.begin() + copyEnd, newOut );

			if ( argStart == fmt.npos )
			{
				return { out, fmt.npos, fmt.npos };
			}

			auto nextPos = argStart + 1;
			if ( nextPos < fmt.size() && fmt[ nextPos ] == '{' )
			{
				*( out++ ) = '{';
				copyStart = argStart + 2;
				continue;
			}

			auto rightBracePos = fmt.find_first_of( '}', argStart + 1 );
			if ( rightBracePos == fmt.npos )
			{
				dbBreak();
				throw format_exception( "missing end of format specifier" );
			}

			return { out, argStart, rightBracePos + 1 };
		}
	}

	template <typename CharT>
	inline std::pair<size_t, size_t> format_copy_str_return_argpos(
		std::basic_string<CharT>& str,
		std::basic_string_view<CharT> fmt )
	{
		size_t copyStart = 0;
		while ( true )
		{
			const size_t argStart = fmt.find_first_of( '{', copyStart );

			const size_t copyEnd = std::min( fmt.size(), argStart );
			str.insert( str.end(), fmt.begin() + copyStart, fmt.begin() + copyEnd );

			if ( argStart == fmt.npos )
			{
				return { fmt.npos, fmt.npos };
			}

			auto nextPos = argStart + 1;
			if ( nextPos < fmt.size() && fmt[ nextPos ] == '{' )
			{
				str += '{';
				copyStart = argStart + 2;
				continue;
			}

			auto rightBracePos = fmt.find_first_of( '}', argStart + 1 );
			if ( rightBracePos == fmt.npos )
			{
				dbBreak();
				throw format_exception( "missing end of format specifier" );
			}

			return { argStart, rightBracePos + 1 };
		}
	}

	template <typename CharT>
	inline void format_argument( std::basic_string<CharT>& outstr, std::basic_string_view<CharT> specifier, CharT c )
	{
		(void)specifier; // TODO

		outstr += c;
	}

	template <typename CharT>
	inline void format_argument(
		std::basic_string<CharT>& outstr,
		std::basic_string_view<CharT> specifier,
		std::basic_string_view<CharT> str )
	{
		(void)specifier; // TODO

		outstr += str;
	}

	template <typename CharT>
	inline void format_argument(
		std::basic_string<CharT>& outstr,
		std::basic_string_view<CharT> specifier,
		std::basic_string<CharT> str )
	{
		(void)specifier; // TODO

		outstr += str;
	}

	template <typename CharT>
	inline void format_argument(
		std::basic_string<CharT>& outstr,
		std::basic_string_view<CharT> specifier,
		const CharT* str )
	{
		(void)specifier; // TODO

		outstr += str;
	}

	template <typename CharT, typename Arg>
	inline void format_argument(
		std::basic_string<CharT>& outstr,
		std::basic_string_view<CharT> specifier,
		const Arg& arg )
	{
		(void)specifier; // TODO

		if constexpr ( std::is_convertible_v<Arg, std::basic_string_view<CharT>> )
		{
			format_argument( outstr, specifier, std::basic_string_view<CharT> { arg } );
		}
		else
		{
			std::basic_stringstream<CharT> ss;
			ss << arg;
			outstr += ss.str();
		}
	}

	template <typename OutputIt, typename CharT>
	inline OutputIt format_argument_to_n(
		OutputIt out,
		it_diff_t<OutputIt> n,
		std::basic_string_view<CharT> specifier,
		CharT c )
	{
		(void)specifier; // TODO

		if ( n > 0 )
			*( out++ ) = c;

		return out;
	}

	template <typename OutputIt, typename CharT>
	inline OutputIt format_argument_to_n(
		OutputIt out,
		it_diff_t<OutputIt> n,
		std::basic_string_view<CharT> specifier,
		std::basic_string_view<CharT> str )
	{
		(void)specifier; // TODO

		return std::copy_n( str.begin(), std::min( str.size(), n ), out );
	}

	template <typename OutputIt, typename CharT>
	inline OutputIt format_argument_to_n(
		OutputIt out,
		it_diff_t<OutputIt> n,
		std::basic_string_view<CharT> specifier,
		std::basic_string<CharT> str )
	{
		(void)specifier; // TODO

		return std::copy_n( str.begin(), std::min( str.size(), n ), out );
	}

	template <typename OutputIt, typename CharT>
	inline OutputIt format_argument_to_n(
		OutputIt out,
		it_diff_t<OutputIt> n,
		std::basic_string_view<CharT> specifier,
		const CharT* str )
	{
		return format_argument_to_n( out, n, specifier, std::basic_string_view<CharT>( str ) );
	}

	template <typename OutputIt, typename CharT, typename Arg>
	inline OutputIt format_argument_to_n(
		OutputIt out,
		it_diff_t<OutputIt> n,
		std::basic_string_view<CharT> specifier,
		const Arg& arg )
	{
		(void)specifier; // TODO

		if constexpr ( std::is_convertible_v<Arg, std::basic_string_view<CharT>> )
		{
			return format_argument_to_n( out, n, specifier, std::basic_string_view<CharT>{ arg } );
		}
		else
		{
			std::basic_stringstream<CharT> ss;
			ss << arg;
			const auto str = ss.str();
			return std::copy_n( str.begin(), std::min( str.size(), static_cast<size_t>( n ) ), out );
		}
	}

	template <typename CharT>
	inline void format_imp( std::basic_string<CharT>& outstr, std::basic_string_view<CharT> fmt )
	{
		auto[ argStart, argEnd ] = format_copy_str_return_argpos( outstr, fmt );

		if ( argStart != fmt.npos )
		{
			dbBreak();
			throw format_exception( "too few format arguments" );
		}
	}

	template <typename CharT, typename Arg1, typename... Args>
	inline void format_imp(
		std::basic_string<CharT>& outstr,
		std::basic_string_view<CharT> fmt,
		const Arg1& arg1,
		const Args&... args )
	{
		auto[ argStart, argEnd ] = format_copy_str_return_argpos( outstr, fmt );
		format_argument( outstr, fmt.substr( argStart + 1, argEnd - argStart - 2 ), arg1 );
		format_imp( outstr, fmt.substr( argEnd ), args... );
	}

	template <typename OutputIt, typename CharT>
	inline OutputIt format_to_n_imp(
		OutputIt out,
		it_diff_t<OutputIt> n,
		std::basic_string_view<CharT> fmt )
	{
		auto[ newOut, argStart, argEnd ] = format_to_and_return_argpos( out, n, fmt );

		if ( argStart != fmt.npos )
		{
			dbBreak();
			throw format_exception( "too few format arguments" );
		}

		return newOut;
	}

	template <typename OutputIt, typename CharT, typename Arg1, typename... Args>
	inline OutputIt format_to_n_imp(
		const OutputIt out,
		it_diff_t<OutputIt> n,
		std::basic_string_view<CharT> fmt,
		const Arg1& arg1,
		const Args&... args )
	{
		auto[ out1, argStart, argEnd ] = format_copy_n_return_argpos( out, n, fmt );
		n -= std::distance( out, out1 );

		if ( n == 0 )
			return out1;

		auto out2 = format_argument_to_n( out1, n, fmt.substr( argStart + 1, argEnd - argStart - 2 ), arg1 );
		n -= std::distance( out, out2 );

		if ( n == 0 )
			return out2;

		return format_to_n_imp( out2, n, fmt.substr( argEnd ), args... );
	}

} // namespace detail

template <typename OutputIt, typename... Args>
inline OutputIt format_to( OutputIt out, std::string_view fmt, const Args&... args )
{
	return detail::format_to_n_imp(
		out,
		std::numeric_limits<detail::it_diff_t<OutputIt>>::max(),
		fmt,
		args... );
}

template <typename OutputIt, typename... Args>
inline OutputIt format_to( OutputIt out, std::wstring_view fmt, const Args&... args )
{
	return detail::format_to_n_imp(
		out,
		std::numeric_limits<detail::it_diff_t<OutputIt>>::max(),
		fmt,
		args... );
}

template <typename OutputIt, typename... Args>
inline OutputIt format_to_n(
	OutputIt out,
	detail::it_diff_t<OutputIt> n,
	std::string_view fmt,
	const Args&... args )
{
	return detail::format_to_n_imp( out, n, fmt, args... );
}

template <typename OutputIt, typename... Args>
inline OutputIt format_to_n(
	OutputIt out,
	detail::it_diff_t<OutputIt> n,
	std::wstring_view fmt,
	const Args&... args )
{
	return detail::format_to_n_imp( out, n, fmt, args... );
}

template <typename... Args>
inline std::string format( std::string_view fmt, const Args&... args )
{
	std::string str;
	str.reserve( fmt.size() );
	detail::format_imp( str, fmt, args... );
	return str;
}

template <typename... Args>
inline std::wstring format( std::wstring_view fmt, const Args&... args )
{
	std::wstring str;
	str.reserve( fmt.size() );
	detail::format_imp( str, fmt, args... );
	return str;
}

namespace detail
{
	template <typename CharT, typename Traits>
	inline void format_read_str( std::basic_istream<CharT, Traits>& in, std::basic_string_view<CharT, Traits> str )
	{
		for ( auto c : str )
		{
			if ( in.peek() == c )
			{
				in.get();
			}
			else
			{
				in.setstate( std::ios::failbit );
				break;
			}
		}
	}
}

template <typename CharT, typename Traits>
void format_read( std::basic_istream<CharT, Traits>& in, std::basic_string_view<CharT, Traits> formatStr )
{
	const auto specifier = detail::find_format_specifier( formatStr );
	if ( specifier.first != formatStr.npos )
		throw format_exception( "too few arguments supplied" );

	detail::format_read_str( in, formatStr );
}

template <typename CharT, typename Traits, typename Head, typename... Tail>
void format_read( std::basic_istream<CharT, Traits>& in, std::basic_string_view<CharT, Traits> formatStr, Head& head, Tail&... tail )
{
	const auto specifier = detail::find_format_specifier( formatStr );

	if ( specifier.first == formatStr.npos )
		throw format_exception( "too many arguments supplied" );

	detail::format_read_str( in, formatStr.substr( 0, specifier.first ) );
	if ( in.fail() )
		return;

	in >> head;

	format_read( in, formatStr.substr( specifier.second ), tail... );
}

} // namespace stdx

#endif