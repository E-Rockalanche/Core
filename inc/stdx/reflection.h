#pragma once

#include "stdx/assert.h"
#include "stdx/ctype.h"
#include "stdx/string.h"
#include "stdx/zstring_view.h"

#include <array>

namespace stdx {
namespace reflection {

namespace detail {

	template <std::size_t N>
	class static_string
	{
	public:
		constexpr static_string( const std::string_view str ) noexcept : static_string( str.data() ) { dbExpects( str.size() == N ); }

		constexpr bool empty() const noexcept { return N == 0; }
		constexpr std::size_t size() const noexcept { return N; }
		constexpr const char* data() const noexcept { return m_data.data(); }
		constexpr const char* c_str() const noexcept { return m_data.data(); }
		constexpr operator stdx::zstring_view() const noexcept { return stdx::zstring_view{ data(), size() }; }
		constexpr const char& operator[]( std::size_t index ) const noexcept { return m_data[ index ]; }

	private:
		constexpr static_string( const char* str ) noexcept { char_traits<char>::copy( m_data.data(), str, N ); }

	private:
		std::array<char, N + 1> m_data{};
	};

	constexpr std::string_view find_name_from_end( std::string_view str ) noexcept
	{
		for ( std::size_t i = str.size(); i-- > 0; )
		{
			const char c = str[ i ];
			if ( !( isalnum( c ) || c == '_' ) )
			{
				str.remove_prefix( i + 1 );
				break;
			}
		}

		if ( str.empty() || isdigit( str.front() ) )
			return {};

		return str;
	}

	template <typename T>
	constexpr auto extract_type_name() noexcept
	{
		constexpr std::string_view name = find_name_from_end( { __FUNCSIG__, sizeof( __FUNCSIG__ ) - 17 } );
		return static_string<name.size()>{ name };
	}

	template <typename T, T Value>
	constexpr auto extract_value_name() noexcept
	{
		constexpr std::string_view name = find_name_from_end( { __FUNCSIG__, sizeof( __FUNCSIG__ ) - 17 } );
		return static_string<name.size()>{ name };
	}

} // namespace detail

template <typename T>
constexpr auto type_name_v = detail::extract_type_name<T>();

template <typename T, T Value>
constexpr auto value_name_v = detail::extract_value_name<T, Value>();

} // namespace reflection
} // namespace stdx