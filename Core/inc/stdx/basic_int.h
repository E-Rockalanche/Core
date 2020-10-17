#pragma once

namespace stdx
{

template <size_t Bits, bool Signed>
class basic_int
{
private:
	using high64_t = std::conditional_t<Signed, int64_t, uint64_t>;



public:

	// construction/assignment

	basic_int() noexcept = default;

	template <typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
		constexpr basic_int( T value ) noexcept : m_low{ value }, m_high{ ( value < 0 ) ? (high64_t)-1 : 0 } {}

	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
		constexpr basic_int( T value ) noexcept : m_low( value ), m_high( value / std::numeric_limits<uint64_t>::max() ) {}

	constexpr basic_int( const basic_int& ) noexcept = default;

	constexpr basic_int& operator=( const basic_int& ) noexcept = default;

	// conversion

	constexpr bool operator!() const noexcept
	{
		return *this <= 0;
	}

	operator bool() const noexcept
	{
		return *this > 0;
	}

	template <typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
		operator T() const noexcept
	{
		return static_cast<T>( m_low );
	}

	// comparison

	friend constexpr bool operator==( const basic_int& lhs, const basic_int& rhs )
	{
		return lhs.m_low == rhs.m_low && lhs.m_high == rhs.m_high;
	}

	friend constexpr bool operator==( const basic_int& lhs, const basic_int& rhs )
	{
		return !( lhs == rhs );
	}

	friend constexpr bool operator<( const basic_int& lhs, const basic_int& rhs )
	{
		if ( lhs.m_high != rhs.m_high )
			return lhs.m_high < rhs.m_high;

		return lhs.m_low < rhs.m_low;
	}

	friend constexpr bool operator>( const basic_int& lhs, const basic_int& rhs )
	{
		return rhs < lhs;
	}

	friend constexpr bool operator<=( const basic_int& lhs, const basic_int& rhs )
	{
		return !( lhs > rhs );
	}

	friend constexpr bool operator>=( const basic_int& lhs, const basic_int& rhs )
	{
		return !( lhs < rhs );
	}

	// math operators

	constexpr const basic_int& operator+() const noexcept
	{
		return *this;
	}

	constexpr basic_int operator-() const noexcept
	{
		return basic_int{ ~m_low, ~m_high } +1;
	}

	constexpr basic_int& operator++() noexcept
	{
		return operator+=( 1 );
	}

	constexpr basic_int operator++( int ) noexcept
	{
		basic_int temp = *this;
		operator++();
		return temp;
	}

	constexpr basic_int& operator--() noexcept
	{
		return operator+=( -1 );
	}

	constexpr basic_int operator--( int ) noexcept
	{
		basic_int temp = *this;
		operator--();
		return temp;
	}

	constexpr basic_int& operator+=( const basic_int& rhs ) noexcept
	{
		const uint64_t low = lhs.m_low + rhs.m_low;
		const bool overflow = ( ( ( lhs.m_low | rhs.m_low ) ^ low ) & 0x8000000000000000 ) != 0;
		m_low = low;
		m_high = lhs.m_high + rhs.m_high + static_cast<high64_t>( overflow );
		return *this;
	}

	constexpr basic_int& operator-=( const basic_int& rhs ) noexcept
	{
		return operator+=( -rhs );
	}

	constexpr basic_int& operator*=( const basic_int& rhs ) noexcept
	{
		const uint64_t low_overflow = ( ( m_low >> 32 ) * ( rhs.m_low >> 32 ) ) >> 32;
		m_high = ( m_high * rhs.m_low ) + ( m_low * rhs.m_high ) + static_cast<high64_t>( low_overflow );
		m_low *= rhs.m_low;
		return *this;
	}

	constexpr basic_int& operator/=( const basic_int& rhs ) noexcept
	{

	}

	friend constexpr basic_int operator+( basic_int lhs, const basic_int& rhs ) noexcept
	{
		return lhs += rhs;
	}

	friend constexpr basic_int operator-( basic_int lhs, const basic_int& rhs ) noexcept
	{
		return lhs -= rhs;
	}

	friend constexpr basic_int operator*( basic_int lhs, const basic_int& rhs ) noexcept
	{
		return lhs *= rhs;
	}

	// bitwise operators

	constexpr basic_int& operator~() const noexcept
	{
		return basic_int( ~m_low, ~m_high );
	}

	constexpr basic_int& operator&=( const basic_int& rhs ) noexcept
	{
		m_low &= rhs.m_low;
		m_high &= rhs.m_high;
	}

	constexpr basic_int& operator|=( const basic_int& rhs ) noexcept
	{
		m_low |= rhs.m_low;
		m_high |= rhs.m_high;
	}

	constexpr basic_int& operator^=( const basic_int& rhs ) noexcept
	{
		m_low ^= rhs.m_low;
		m_high ^= rhs.m_high;
	}

	friend constexpr basic_int operator&( basic_int lhs, const basic_int& rhs ) noexcept
	{
		return lhs &= rhs;
	}

	friend constexpr basic_int operator|( basic_int lhs, const basic_int& rhs ) noexcept
	{
		return lhs |= rhs;
	}

	friend constexpr basic_int operator^( basic_int lhs, const basic_int& rhs ) noexcept
	{
		return lhs ^= rhs;
	}

	// bitshift operators

	constexpr basic_int& operator<<=( std::ptrdiff_t n ) noexcept
	{
		if ( n < 0 )
			return operator>>=( -n );

		m_high = ( m_high << n ) | ( m_low << ( n - 64 ) );
		m_low <<= n;
	}

	constexpr basic_int& operator>>=( std::ptrdiff_t n ) noexcept
	{
		if ( n < 0 )
			return operator<<=( -n );

		m_low = ( m_low >> n ) | ( m_high >> ( n - 64 ) );
		m_high >>= n;
	}

	friend basic_int operator<<( basic_int lhs, std::ptrdiff_t rhs ) noexcept
	{
		return lhs <<= rhs;
	}

	friend basic_int operator>>( basic_int lhs, std::ptrdiff_t rhs ) noexcept
	{
		return lhs >>= rhs;
	}

private:
	constexpr basic_int( uint64_t low, high64_t high ) noexcept : m_low{ low }, m_high{ high } {}

	// little endian
	uint64_t m_low;
	high64_t m_high;
};

}