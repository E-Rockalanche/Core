#pragma once

namespace stdx
{

	template <typename CharT>
	struct char_traits
	{
		using char_type = CharT;
		using int_type = int;

		static constexpr void assign( char_type& r, const char_type a ) noexcept
		{
			r = a;
		}

		static constexpr char_type* assign( char_type* const p, const std::size_t count, const char_type a )
		{
			for ( std::size_t i = 0; i < count; ++i )
			{
				assign( p[ i ], a );
			}
			return p;
		}

		static constexpr bool eq( const char_type a, const char_type b ) noexcept
		{
			return a == b;
		}

		static constexpr bool lt( const char_type a, const char_type b ) noexcept
		{
			return a < b;
		}

		static constexpr char_type* move( char_type* const dest, const char_type* const src, const std::size_t count )
		{
			if ( dest < src )
			{
				for ( std::size_t i = 0; i < count; ++i )
				{
					assign( dest[ i ], src[ i ] );
				}
			}
			else
			{
				for ( std::size_t i = count; i-- > 0; )
				{
					assign( dest[ i ], src[ i ] );
				}
			}
			return dest;
		}

		static constexpr char_type* copy( char_type* const dest, const char_type* const src, const std::size_t count )
		{
			for ( std::size_t i = 0; i < count; ++i )
			{
				assign( dest[ i ], src[ i ] );
			}
			return dest;
		}

		static constexpr int compare( const char_type* const lhs, const char_type* const rhs, const std::size_t count )
		{
			for ( std::size_t i = 0; i < count; ++i )
			{
				if ( lt( lhs[ i ], rhs[ i ] ) )
					return -1;
				else if ( lt( rhs[ i ], lhs[ i ] ) )
					return +1;
				else if ( lhs[ i ] == CharT( 0 ) && rhs[ i ] == CharT( 0 ) )
					return 0;
			}
			return 0;
		}

		static constexpr std::size_t length( const char_type* str )
		{
			std::size_t l = 0;
			while ( str[ l ] != CharT( 0 ) )
				++l;
			return l;
		}

		static constexpr const char_type* find( const char_type* str, const std::size_t count, const char_type c )
		{
			for ( std::size_t i = 0; i < count; ++i )
			{
				if ( eq( str[ i ], c ) )
					return str + i;
			}
			return nullptr;
		}

		static constexpr char_type to_char_type( const int_type i ) noexcept
		{
			return static_cast<char_type>( i );
		}

		static constexpr int_type to_int_type( const char_type c ) noexcept
		{
			return static_cast<int_type>( c );
		}

		static constexpr bool eq_int_type( int_type c1, int_type c2 ) noexcept
		{
			return eq( to_char_type( c1 ), to_char_type( c2 ) );
		}

		static constexpr int_type eof() noexcept { return (int_type)-1; }

		static constexpr int_type not_eof( int_type e ) noexcept
		{
			return eq_int_type( e, eof() ) ? 0 : e;
		}
	};

}