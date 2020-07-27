#pragma once

#include <stdx/bit.h>
#include <stdx/math.h>
#include <stdx/type_traits.h>

#include <limits>

namespace Math
{

template <typename B, int P>
class FixedPoint
{
public:
	struct BaseTypeTag {};

	using base_type = B;
	using ubase_type = std::make_unsigned_t<base_type>;

	static_assert( std::is_integral_v<B> && !std::is_same_v<B, bool> );
	static_assert( 0 < P && P < std::numeric_limits<ubase_type>::digits );

	static constexpr int precision = P;
	static constexpr int digits = std::numeric_limits<ubase_type>::digits - precision;

	static constexpr base_type scale = 1 << precision;

	static constexpr ubase_type fraction_mask = static_cast<ubase_type>( scale - 1 );
	static constexpr ubase_type whole_mask = ~fraction_mask;

	static constexpr base_type max_base = std::numeric_limits<base_type>::max();
	static constexpr base_type min_base = std::numeric_limits<base_type>::min();

	static constexpr base_type max_whole = max_base / scale;
	static constexpr base_type min_whole = min_base / scale;
	
	// construction

	FixedPoint() noexcept = default;

	constexpr FixedPoint( const FixedPoint& ) noexcept = default;

	constexpr FixedPoint( base_type x ) noexcept : m_value{ stdx::safe_multiply( x, scale ) } {}

	template <typename T,
		std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
	explicit constexpr FixedPoint( T x ) noexcept : m_value{ static_cast<B>( x * scale ) }
	{
		dbExpects( min_whole <= x && x <= max_whole );
	}

	explicit constexpr FixedPoint( base_type x, BaseTypeTag ) noexcept : m_value{ x } {}

	// conversion

	constexpr B Base() const noexcept { return m_value; }

	template <typename T>
	constexpr operator T() const noexcept
	{
		if constexpr ( std::is_floating_point_v<T> )
			return static_cast<T>( m_value ) / scale;
		else
			return stdx::narrow_cast<T>( m_value / scale );
	}

	// assignment

	constexpr FixedPoint& operator=( const FixedPoint& ) noexcept = default;

	constexpr FixedPoint& operator+=( const FixedPoint& x ) noexcept
	{
		m_value += x.m_value;
		return *this;
	}

	constexpr FixedPoint& operator-=( const FixedPoint& x ) noexcept
	{
		m_value -= x.m_value;
		return *this;
	}

	constexpr FixedPoint& operator*=( const FixedPoint& x ) noexcept
	{
		m_value = Multiply( m_value, x.m_value );
		return *this;
	}

	constexpr FixedPoint& operator*=( base_type x ) noexcept
	{
		m_value = stdx::safe_multiply( m_value, x );
		return *this;
	}

	constexpr FixedPoint& operator/=( const FixedPoint& x ) noexcept
	{
		m_value = Divide( m_value, x.m_value );
		return *this;
	}

	constexpr FixedPoint& operator/=( base_type x ) noexcept
	{
		m_value /= x;
		return *this;
	}

private:
	constexpr B Multiply( B lhs, B rhs ) noexcept;
	constexpr B Divide( B lhs, B rhs ) noexcept;

private:
	B m_value;
};

template <typename B, int P>
constexpr B FixedPoint<B, P>::Multiply( B lhs, B rhs ) noexcept
{
	if constexpr ( stdx::is_detected_v<stdx::promoted_fast_t, B> )
	{
		using temp_type = stdx::promoted_fast_t<B>;
		return stdx::narrow_cast<B>( ( static_cast<temp_type>( lhs ) * static_cast<temp_type>( rhs ) ) / scale );
	}
	else
	{
		// backup if B cannot be promoted

		using UB = std::make_unsigned_t<B>;
		static constexpr auto half_digits = std::numeric_limits<UB>::digits / 2;
		static constexpr UB low_mask = std::numeric_limits<UB>::max() >> half_digits;

		const UB lhs_frac = static_cast<UB>( lhs ) & fraction_mask;
		const UB lhs_whole = static_cast<UB>( lhs ) >> precision;

		const UB rhs_frac = static_cast<UB>( rhs ) & fraction_mask;
		const UB rhs_whole = static_cast<UB>( rhs ) >> precision;

		const auto r1 = stdx::safe_multiply( lhs_frac, rhs_frac ) / scale;
		const auto r2 = stdx::safe_multiply( lhs_frac, rhs_whole );
		const auto r3 = stdx::safe_multiply( rhs_frac, lhs_whole );
		const auto r4 = stdx::safe_multiply( stdx::safe_multiply( lhs_whole, rhs_whole ), scale );

		const auto r12 = stdx::safe_add( r1, r2 );
		const auto r34 = stdx::safe_add( r3, r4 );
		return stdx::safe_add( r12, r34 );
	}
}

template <typename B, int P>
constexpr B FixedPoint<B, P>::Divide( B lhs, B rhs ) noexcept
{
	using temp_type = stdx::promoted_fast_t<B>;
	return stdx::narrow_cast<B>( ( static_cast<temp_type>( lhs ) * static_cast<temp_type>( scale ) ) / rhs );
}

template <typename B, int P>
constexpr FixedPoint<B, P> operator+( FixedPoint<B, P> x ) noexcept
{
	return x;
}

template <typename B, int P>
constexpr FixedPoint<B, P> operator-( FixedPoint<B, P> x ) noexcept
{
	return x * stdx::narrow_cast<B>( -1 );
}

template <typename B, int P>
constexpr FixedPoint<B, P> operator+( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs += rhs;
}

template <typename B, int P>
constexpr FixedPoint<B, P> operator-( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs -= rhs;
}

template <typename B, int P>
constexpr FixedPoint<B, P> operator*( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs *= rhs;
}

template <typename B, int P>
constexpr FixedPoint<B, P> operator*( FixedPoint<B, P> lhs, B rhs ) noexcept
{
	return lhs *= rhs;
}

template <typename B, int P>
constexpr FixedPoint<B, P> operator*( B lhs, FixedPoint<B, P> rhs ) noexcept
{
	return rhs *= lhs;
}

template <typename B, int P>
constexpr FixedPoint<B, P> operator/( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs /= rhs;
}

template <typename B, int P>
constexpr FixedPoint<B, P> operator/( FixedPoint<B, P> lhs, B rhs ) noexcept
{
	return lhs /= rhs;
}

// comparisons

template <typename B, int P>
constexpr bool operator==( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs.Base() == rhs.Base();
}

template <typename B, int P>
constexpr bool operator!=( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs.Base() != rhs.Base();
}

template <typename B, int P>
constexpr bool operator<( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs.Base() < rhs.Base();
}

template <typename B, int P>
constexpr bool operator>( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs.Base() > rhs.Base();
}

template <typename B, int P>
constexpr bool operator<=( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs.Base() <= rhs.Base();
}

template <typename B, int P>
constexpr bool operator>=( FixedPoint<B, P> lhs, FixedPoint<B, P> rhs ) noexcept
{
	return lhs.Base() >= rhs.Base();
}

} // namespace Math

namespace std
{

template <typename B, int P>
struct hash<Math::FixedPoint<B, P>>
{
	constexpr auto operator()( Math::FixedPoint<B, P> x ) noexcept
	{
		return hash<B>{}( x.Base() );
	}
};

template <typename B, int P>
class numeric_limits<Math::FixedPoint<B, P>>
{
	static constexpr bool is_specialized = true;
	static constexpr bool is_signed = std::is_signed_v<B>;
	static constexpr bool is_integer = false;
	static constexpr bool is_exact = false;
	static constexpr bool has_infinity = false;
	static constexpr bool has_quiete_NaN = false;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;
	static constexpr float_round_style round_style = round_toward_zero;
	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = true;
	static constexpr int digits = numeric_limits<B>::digits;
	static constexpr int digits10 = numeric_limits<B>::digits10;
	static constexpr int max_digits10 = numeric_limits<B>::max_digits10;
	static constexpr int radix = numeric_limits<B>::radix;
	static constexpr int min_exponent = 0;
	static constexpr int min_exponent10 = 0;
	static constexpr int max_exponent = 0;
	static constexpr int max_exponent10 = 0;
	static constexpr bool traps = true;
	static constexpr bool tinyness_before = false;

	static constexpr Math::FixedPoint<B, P> lowest() noexcept
	{
		return Math::FixedPoint{ numeric_limits<B>::lowest(), Math::FixedPoint<B, P>::BaseTypeTag{} };
	}

	static constexpr Math::FixedPoint<B, P> (min)() noexcept
	{
		return Math::FixedPoint{ 1, Math::FixedPoint::BaseTypeTag{} };
	}

	static constexpr Math::FixedPoint<B, P> (max)() noexcept
	{
		return Math::FixedPoint{ (numeric_limits<B>::max)(), Math::FixedPoint<B, P>::BaseTypeTag{} };
	}

	static constexpr Math::FixedPoint<B, P> epsilon() noexcept
	{
		return Math::FixedPoint{ 1, Math::FixedPoint::BaseTypeTag{} };
	}

	static constexpr Math::FixedPoint<B, P> round_error() noexcept
	{
		return Math::FixedPoint{ Math::FixedPoint<B, P>::scale / 2, Math::FixedPoint::BaseTypeTag{} };
	}
};

} // namespace std

namespace Math
{

using fx32 = FixedPoint<int32_t, 10>;
using fx64 = FixedPoint<int32_t, 32>;

}