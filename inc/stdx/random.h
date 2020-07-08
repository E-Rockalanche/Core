#pragma once

#include <stdx/bit.h>

#include <random>

namespace stdx {

template <size_t StateSize = 4, uint32_t A = 987654366>
class complimentary_multiply_with_carry_engine
{
public:
	static_assert( StateSize > 0, "state size must be larger than 0" );

	using result_type = uint32_t;

	static constexpr result_type min() noexcept { return 0; }
	static constexpr result_type max() noexcept { return static_cast<result_type>( -1 ); }

	static constexpr uint64_t multiplier = A;
	static constexpr result_type default_seed = 0;

	constexpr complimentary_multiply_with_carry_engine() noexcept
		: complimentary_multiply_with_carry_engine( default_seed )
	{}

	constexpr explicit complimentary_multiply_with_carry_engine( result_type seed_ ) noexcept
	{
		seed( seed_ );
	}

	constexpr void seed( result_type seed_ = default_seed ) noexcept
	{
		std::seed_seq seq{ seed_ };
		seq.generate( std::begin( m_state ), std::end( m_state ) );
		m_carry = static_cast<uint32_t>( ( m_state[ StateSize - 1 ] * multiplier ) >> 32 );
	}

	constexpr result_type operator()() noexcept
	{
		m_index = ( m_index + 1 ) % StateSize;
		const uint64_t t = multiplier * m_state[ m_index ] + m_carry;
		m_carry = static_cast<uint32_t>( t >> 32 );
		const uint32_t x = static_cast<uint32_t>( t ) + m_carry;
		if ( x < m_carry )
		{
			++x;
			++m_carry;
		}
		return m_state[ m_index ] = 0xfffffffe - x;
	}

	constexpr void discard( unsigned long long z ) noexcept
	{
		for ( ; z-- > 0; )
		{
			(void)operator();
		}
	}

	friend constexpr bool operator==( const complimentary_multiply_with_carry_engine& lhs, const complimentary_multiply_with_carry_engine& rhs ) noexcept
	{
		if ( lhs.m_carry != rhs.m_carry || lhs.m_index != rhs.m_index )
			return false;

		const auto[ lhsIt, rhsIt ] = std::mismatch( std::begin( lhs.m_state ), std::end( lhs.m_state ), std::begin( rhs.m_state ), std::end( rhs.m_state ) );
		dbEnsures( ( lhsIt == std::end( lhs.m_state ) ) == ( rhsIt == std::end( rhs.m_state ) ) );
		return lhsIt == std::end( lhs.m_state );
	}

	friend constexpr bool operator!=( const complimentary_multiply_with_carry_engine& lhs, const complimentary_multiply_with_carry_engine& rhs ) noexcept
	{
		return !( lhs == rhs );
	}

	template< class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<( std::basic_ostream<CharT, Traits>& os, const complimentary_multiply_with_carry_engine& rhs )
	{
		for ( auto& value : m_state )
		{
			os << value << ' ';
		}
		os << m_carry << ' ' << m_index;
		return os;
	}

	template< class CharT, class Traits>
	friend std::basic_istream<CharT, Traits>& operator>>( std::basic_istream<CharT, Traits>& is, complimentary_multiply_with_carry_engine& rhs )
	{
		for ( auto& value : m_state )
		{
			is >> value;
		}
		is >> m_carry >> m_index;
		return is;
	}

private:
	uint32_t m_state[ StateSize ]{};
	uint32_t m_carry = 0;
	uint32_t m_index = StateSize - 1;
};

template <typename Generator>
class rng_wrapper
{
public:
	using result_type = typename Generator::result_type;

	constexpr rng_wrapper() noexcept = default;

	constexpr rng_wrapper( uint32_t seed ) noexcept : m_generator{ seed } {}

	constexpr result_type operator()() noexcept
	{
		return m_generator();
	}

	/// Produces random integer values i, uniformly distributed on the closed interval [a, b]
	template <typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
	constexpr T uniform( T min = 0, T max = std::numeric_limits<T>::max() ) noexcept
	{
		static_assert( sizeof( T ) > 1, "undefined behaviour" );
		std::uniform_int_distribution<T> dist( min, max );
		return dist( m_generator );
	}

	/// Produces random floating-point values i, uniformly distributed on the interval [a, b)
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
		constexpr T uniform( T min = 0, T max = 1 ) noexcept
	{
		std::uniform_real_distribution<T> dist( min, max );
		return dist( m_generator );
	}

	/// Produces random boolean values, according to the discrete probability function
	constexpr bool bernoulli( double p = 0.5 ) noexcept
	{
		std::bernoulli_distribution dist( p );
		return dist( m_generator );
	}

	/// Produces a value which represents the number of successes in a sequence of t yes/no experiments, each of which succeeds with probability p
	template <typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
	constexpr T binomial( T t, double p = 0.5 ) noexcept
	{
		static_assert( sizeof( T ) > 1, "undefined behaviour" );
		std::binomial_distribution<T> dist( t, p );
		return dist( m_generator );
	}

	/// Produces a value which represents the number of failures in a series of independent yes/no trials (each succeeds with probability p), before exactly k successes occur
	template <typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
	constexpr T negative_binomial( T k, double p = 0.5 ) noexcept
	{
		static_assert( sizeof( T ) > 1, "undefined behaviour" );
		std::negative_binomial_distribution<T> dist( k, p );
		return dist( m_generator );
	}

	/// Produces a value which represents the number of yes/no trials (each succeeding with probability p) which are necessary to obtain a single success
	template <typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
	constexpr T geometric( double probability = 0.5 ) noexcept
	{
		static_assert( sizeof( T ) > 1, "undefined behaviour" );
		std::geometric_distribution<T> dist( probability );
		return dist( m_generator );
	}

	/// Produces a value which represents the probability of exactly i occurrences of a random event if the expected, mean number of its occurrence under the same conditions (on the same time/space interval) is u
	template <typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
	constexpr T poisson( double mean = 1 ) noexcept
	{
		static_assert( sizeof( T ) > 1, "undefined behaviour" );
		std::poisson_distribution<T> dist( mean );
		return dist( m_generator );
	}

	/// Produces a value which represents the time/distance until the next random event if random events occur at constant rate lambda per unit of time/distance. For example, this distribution describes the time between the clicks of a Geiger counter or the distance between point mutations in a DNA strand
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T exponential( T lambda = 1 ) noexcept
	{
		std::exponential_distribution<T> dist( lambda );
		return dist( m_generator );
	}

	/// For floating-point alpha, the value obtained is the sum of alpha independent exponentially distributed random variables, each of which has a mean of beta
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T gamma( T alpha = 1, T beta = 1 ) noexcept
	{
		std::gamma_distribution<T> dist( alpha, beta );
		return dist( m_generator );
	}

	/// produces random numbers according to the Weibull distribution
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T weibull( T a = 1, T b = 1 ) noexcept
	{
		std::weibull_distribution<T> dist( a, b );
		return dist( m_generator );
	}

	/// Produces random numbers according to the extreme value distribution (it is also known as Gumbel Type I, log-Weibull, Fisher-Tippett Type I)
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T extreme_value( T a = 0, T b = 1 ) noexcept
	{
		std::extreme_value_distribution<T> dist( a, b );
		return dist( m_generator );
	}

	/// Generates random numbers according to the Normal (or Gaussian) random number distribution
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T normal( T mean = 0, T stddev = 1 ) noexcept
	{
		std::normal_distribution<T> dist( mean, stddev );
		return dist( m_generator );
	}

	/// The lognormal_distribution random number distribution produces random numbers x > 0 according to a log-normal distribution
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T lognormal( T mean = 0, T stddev = 1 ) noexcept
	{
		std::lognormal_distribution<T> dist( mean, stddev );
		return dist( m_generator );
	}

	/// The chi_squared_distribution produces random numbers x>0 according to the Chi-squared distribution
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T chi_squared( T n = 1 ) noexcept
	{
		std::chi_squared_distribution<T> dist( n );
		return dist( m_generator );
	}

	/// Produces random numbers according to a Cauchy distribution (also called Lorentz distribution)
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T cauchy( T a = 0, T b = 1 ) noexcept
	{
		std::cauchy_distribution<T> dist( a, b );
		return dist( m_generator );
	}

	/// Produces random numbers according to the f-distribution
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T fisher_f( T m = 1, T n = 1 ) noexcept
	{
		std::fisher_f_distribution<T> dist( m, n );
		return dist( m_generator );
	}

	/// This distribution is used when estimating the mean of an unknown normally distributed value given n+1 independent measurements, each with additive errors of unknown standard deviation, as in physical measurements. Or, alternatively, when estimating the unknown mean of a normal distribution with unknown standard deviation, given n+1 samples
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T student_t( T n = 1 ) noexcept
	{
		std::student_t_distribution<T> dist( n );
		return dist( m_generator );
	}

	/// produces random integers on the interval [0, n), where the probability of each individual integer i is defined as w[i] / S, that is the weight of the ith integer divided by the sum of all n weights
	template <typename T, typename InputIt,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
	constexpr T discrete( InputIt first, InputIt last ) noexcept
	{
		std::discrete_distribution<T> dist( first, last );
		return dist( m_generator );
	}

	/// produces random integers on the interval [0, n), where the probability of each individual integer i is defined as w[i] / S, that is the weight of the ith integer divided by the sum of all n weights
	template <typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
	constexpr T discrete( std::initializer_list<double> weights ) noexcept
	{
		std::discrete_distribution<T> dist( weights );
		return dist( m_generator );
	}

	/// produces random integers on the interval [0, n), where the probability of each individual integer i is defined as w[i] / S, that is the weight of the ith integer divided by the sum of all n weights
	template <typename T, typename UnaryOperation,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
	constexpr T discrete( std::size_t count, double xmin, double xmax, UnaryOperation unary_op ) noexcept
	{
		std::discrete_distribution<T> dist( count, xmin, xmax, unary_op );
		return dist( m_generator );
	}

	/// produces random floating-point numbers, which are uniformly distributed within each of the several subintervals [b[i], b[i] + 1), each with its own weight w[i].The set of interval boundaries and the set of weights are the parameters of this distribution
	template <typename T, typename InputIt1, typename InputIt2,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T piecewise_constant( InputIt1 first_i, InputIt1 last_i, InputIt2 first_w ) noexcept
	{
		std::piecewise_constant_distribution<T> dist( first_i, last_i, first_w );
		return dist( m_generator );
	}

	/// produces random floating-point numbers, which are uniformly distributed within each of the several subintervals [b[i], b[i] + 1), each with its own weight w[i].The set of interval boundaries and the set of weights are the parameters of this distribution
	template <typename T, typename UnaryOperation,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T piecewise_constant( std::initializer_list<T> bl, UnaryOperation fw ) noexcept
	{
		std::piecewise_constant_distribution<T> dist( bl, fw );
		return dist( m_generator );
	}

	/// produces random floating-point numbers, which are uniformly distributed within each of the several subintervals [b[i], b[i] + 1), each with its own weight w[i].The set of interval boundaries and the set of weights are the parameters of this distribution
	template <typename T, typename UnaryOperation,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T piecewise_constant( std::size_t nw, T xmin, T xmax, UnaryOperation fw ) noexcept
	{
		std::piecewise_constant_distribution<T> dist( nw, xmin, xmax, fw );
		return dist( m_generator );
	}

	/// produces random floating-point numbers, which are distributed according to a linear probability density function within each of the several subintervals [b[i], b[i] + 1).The distribution is such that the probability density at each interval boundary is exactly the predefined value p[i]
	template <typename T, typename InputIt1, typename InputIt2,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T piecewise_linear( InputIt1 first_i, InputIt1 last_i, InputIt2 first_w ) noexcept
	{
		std::piecewise_linear_distribution<T> dist( first_i, last_i, first_w );
		return dist( m_generator );
	}

	/// produces random floating-point numbers, which are distributed according to a linear probability density function within each of the several subintervals [b[i], b[i] + 1).The distribution is such that the probability density at each interval boundary is exactly the predefined value p[i]
	template <typename T, typename UnaryOperation,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T piecewise_linear( std::initializer_list<T> bl, UnaryOperation fw ) noexcept
	{
		std::piecewise_linear_distribution<T> dist( bl, fw );
		return dist( m_generator );
	}

	/// produces random floating-point numbers, which are distributed according to a linear probability density function within each of the several subintervals [b[i], b[i] + 1).The distribution is such that the probability density at each interval boundary is exactly the predefined value p[i]
	template <typename T, typename UnaryOperation,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	constexpr T piecewise_linear( std::size_t nw, T xmin, T xmax, UnaryOperation fw ) noexcept
	{
		std::piecewise_linear_distribution<T> dist( nw, xmin, xmax, fw );
		return dist( m_generator );
	}

private:
	Generator m_generator;
};

} // namespace stdx