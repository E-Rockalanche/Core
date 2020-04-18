#ifndef STDX_CTYPE_HPP
#define STDX_CTYPE_HPP

namespace stdx {

template <typename CharT>
constexpr bool islower( CharT c ) noexcept
{
	return c >= 'a' && c <= 'z';
}

template <typename CharT>
constexpr bool isupper( CharT c ) noexcept
{
	return c >= 'A' && c <= 'Z';
}

template <typename CharT>
constexpr bool isalpha( CharT c ) noexcept
{
	return stdx::islower( c ) || stdx::isupper( c );
}

template <typename CharT>
constexpr bool isdigit( CharT c ) noexcept
{
	return c >= '0' && c <= '9';
}
	
template <typename CharT>
constexpr bool isalnum( CharT c ) noexcept
{
	return stdx::isalpha( c ) || stdx::isdigit( c );
}

template <typename CharT>
constexpr bool isblank( CharT c ) noexcept
{
	return c == ' ' || c == '\t';
}

template <typename CharT>
constexpr bool isprint( CharT c ) noexcept
{
	return c > 0x1f && c != 0x7f;
}

template <typename CharT>
constexpr bool iscntrl( CharT c ) noexcept
{
	return !stdx::isprint( c );
}

template <typename CharT>
constexpr bool isgraph( CharT c ) noexcept
{
	return c != ' ' && stdx::isprint( c );
}

template <typename CharT>
constexpr bool ispunct( CharT c ) noexcept
{
	return !stdx::isalnum( c ) && stdx::isgraph( c );
}

template <typename CharT>
constexpr bool isspace( CharT c ) noexcept
{
	switch( c )
	{
		case ' ':
		case '\t':
		case '\n':
		case '\v':
		case '\f':
		case '\r':
			return true;

		default:
			return false;
	}
}

template <typename CharT>
constexpr bool isxdigit( CharT c ) noexcept
{
	return stdx::isdigit( c ) ||
		( c >= 'a' && c <= 'f' ) ||
		( c >= 'A' && c <= 'F' );
}

}

#endif