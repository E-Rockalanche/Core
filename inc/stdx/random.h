#pragma once

#include <stdx/bit.h>

#include <algorithm>
#include <limits>
#include <random>

namespace stdx {

template <typename T, std::size_t N,
	std::enable_if_t<std::is_integral_v<T>, int> = 0>
class strict_seed_seq
{
public:
	using value_type = std::array<T, N>;

	constexpr strict_seed_seq( value_type state ) noexcept : m_state{ state } {}

	template <typename Generator>
	constexpr strict_seed_seq( Generator& g ) noexcept
	{
		std::uniform_int_distribution<T> dist;
		for ( auto& value : m_state )
		{
			value = dist( g );
		}
	}

	template <typename It>
	constexpr void generate( It first, It last ) noexcept
	{
		static_assert ( std::is_same_v<T, std::iterator_traits<It>::value_type> );
		dbAssert( std::distance( first, last ) == N );

		(void)last;
		std::copy( m_state.begin(), m_state.end(), first );
	}

	constexpr std::size_t size() const noexcept
	{
		return m_state.size();
	}

private:
	value_type m_state{};
};

template <size_t Lag, uint32_t Multiplier>
class complimentary_multiply_with_carry_engine
{
public:
	static_assert( Lag > 0, "lag must be larger than 0" );

	using result_type = uint32_t;
	using save_state = std::array<uint32_t, Lag + 2>;

	static constexpr result_type min() noexcept { return std::numeric_limits<result_type>::min(); }
	static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

	static constexpr result_type default_seed = 0;

	constexpr complimentary_multiply_with_carry_engine() noexcept
		: complimentary_multiply_with_carry_engine( default_seed )
	{}

	constexpr explicit complimentary_multiply_with_carry_engine( result_type value ) noexcept
	{
		seed( value );
	}

	template <typename SeedSeq>
	constexpr explicit complimentary_multiply_with_carry_engine( SeedSeq&& seq ) noexcept
	{
		seed( seq );
	}

	constexpr explicit complimentary_multiply_with_carry_engine( save_state state ) noexcept
	{
		restore( state );
	}

	constexpr void seed( result_type value = default_seed ) noexcept
	{
		seed( std::seed_seq{ value } );
	}

	template <typename SeedSeq>
	constexpr void seed( SeedSeq&& seq ) noexcept
	{
		seq.generate( std::begin( m_state ), std::end( m_state ) );
	}

	constexpr result_type operator()() noexcept
	{
		m_index = ( m_index + 1 ) % Lag;
		auto& carry = m_state[ Lag ];
		const uint64_t t = static_cast<uint64_t>( Multiplier ) * m_state[ m_index ] + carry;
		carry = static_cast<uint32_t>( t >> 32 );
		uint32_t x = static_cast<uint32_t>( t ) + carry;
		const bool inc = x < carry;
		x += inc;
		carry += inc;
		return m_state[ m_index ] = 0xfffffffe - x;
	}

	constexpr void discard( unsigned long long z ) noexcept
	{
		while( z-- )
		{
			(void)operator();
		}
	}

	friend constexpr bool operator==( const complimentary_multiply_with_carry_engine& lhs, const complimentary_multiply_with_carry_engine& rhs ) noexcept
	{
		if ( lhs.m_index != rhs.m_index )
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
		os << m_index;
		return os;
	}

	template< class CharT, class Traits>
	friend std::basic_istream<CharT, Traits>& operator>>( std::basic_istream<CharT, Traits>& is, complimentary_multiply_with_carry_engine& rhs )
	{
		for ( auto& value : m_state )
		{
			is >> value;
		}
		is >> m_index;
		return is;
	}

	save_state save() const noexcept
	{
		save_state state;
		std::copy( m_state.begin(), m_state.end(), state.begin() );
		state.back() = m_index;
		return state;
	}

	void restore( save_state state ) noexcept
	{
		std::copy( state.begin(), state.end() - 1, m_state.begin() );
		m_index = state.back();
	}

private:
	std::array<uint32_t, Lag + 1> m_state{}; // +1 for carry
	uint32_t m_index = Lag - 1;
};

using cmwc4 = complimentary_multiply_with_carry_engine<4, 987654366>;
using cmwc8 = complimentary_multiply_with_carry_engine<8, 987651386>;
using cmwc16 = complimentary_multiply_with_carry_engine<16, 987651178>;
using cmwc32 = complimentary_multiply_with_carry_engine<32, 987655670>;
using cmwc64 = complimentary_multiply_with_carry_engine<64, 987651206>;
using cmwc128 = complimentary_multiply_with_carry_engine<128, 987688302>;
using cmwc256 = complimentary_multiply_with_carry_engine<256, 987662290>;
using cmwc512 = complimentary_multiply_with_carry_engine<512, 123462658>;
using cmwc1024 = complimentary_multiply_with_carry_engine<1024, 5555698>;
using cmwc2048 = complimentary_multiply_with_carry_engine<2048, 1030770>;



template <typename T>
class uniform_index_distribution : protected std::uniform_int_distribution<T>
{
	struct param_t
	{
		T value;
	};

	uniform_index_distribution() : uniform_index_distribution{ 1 } {}

	uniform_index_distribution( T size ) : std::uniform_int_distribution<T>{ 0, size - 1 }
	{
		dbExpects( size > 0 );
	}

	uniform_index_distribution( param_t p ) : uniform_index_distribution{ p.value } {}

	template <typename Generator>
	T operator()( Generator& g )
	{
		return std::uniform_int_distribution<T>::operator()( g );
	}

	template <typename Generator>
	T operator()( Generator& g, param_t p )
	{
		dbExpects( p.value > 0 );
		return std::uniform_int_distribution<T>::operator()( g, { 0, p - 1 } );
	}
};



/*
rng_wrapper adds standard random distributions as member functions to generator. Should only be used for on-off calls.
Prefer creating distribution objects when generating random numbers in a loop
*/
template <typename Generator>
class rng_wrapper : public Generator
{
public:

	using Generator::Generator;

	/// Produces random integer values i, uniformly distributed on the closed interval [a, b]
	template <typename T,
		std::enable_if_t<std::is_integral_v<T>, int> = 0>
		T uniform( T min = 0, T max = std::numeric_limits<T>::max() )
	{
		std::uniform_int_distribution<T> dist( min, max );
		return dist( *this );
	}

	/// Produces random integer values i, uniformly distributed on the interval [0, size)
	template <typename T>
	T uniform_index( T size )
	{
		std::uniform_int_distribution<T> dist( 0, size - 1 );
		return dist( *this );
	}

	/// Produces random floating-point values i, uniformly distributed on the interval [a, b)
	template <typename T,
		std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
		T uniform( T min = 0.0, T max = 1.0 )
	{
		std::uniform_real_distribution<T> dist( min, max );
		return dist( *this );
	}

	/// Produces random boolean values, according to the discrete probability function
	bool bernoulli( double p = 0.5 )
	{
		// safe to pass any value of p
		std::bernoulli_distribution dist( p );
		return dist( *this );
	}

	/// Produces a value which represents the number of successes in a sequence of t yes/no experiments, each of which succeeds with probability p
	template <typename T>
	T binomial( T t, double p = 0.5 )
	{
		std::binomial_distribution<T> dist( t, p );
		return dist( *this );
	}

	/// Produces a value which represents the number of failures in a series of independent yes/no trials (each succeeds with probability p), before exactly k successes occur
	template <typename T>
	T negative_binomial( T k, double p = 0.5 )
	{
		std::negative_binomial_distribution<T> dist( k, p );
		return dist( *this );
	}

	/// Produces a value which represents the number of yes/no trials (each succeeding with probability p) which are necessary to obtain a single success
	template <typename T>
	T geometric( double p = 0.5 )
	{
		std::geometric_distribution<T> dist( p );
		return dist( *this );
	}

	/// Produces a value which represents the probability of exactly i occurrences of a random event if the expected, mean number of its occurrence under the same conditions (on the same time/space interval) is u
	template <typename T>
	T poisson( double mean = 1.0 )
	{
		std::poisson_distribution<T> dist( mean );
		return dist( *this );
	}

	/// Produces a value which represents the time/distance until the next random event if random events occur at constant rate lambda per unit of time/distance. For example, this distribution describes the time between the clicks of a Geiger counter or the distance between point mutations in a DNA strand
	template <typename T>
	T exponential( T lambda = 1.0 )
	{
		std::exponential_distribution<T> dist( lambda );
		return dist( *this );
	}

	/// For floating-point alpha, the value obtained is the sum of alpha independent exponentially distributed random variables, each of which has a mean of beta
	template <typename T>
	T gamma( T alpha = 1.0, T beta = 1.0 )
	{
		std::gamma_distribution<T> dist( alpha, beta );
		return dist( *this );
	}

	/// produces random numbers according to the Weibull distribution
	template <typename T>
	T weibull( T a = 1.0, T b = 1.0 )
	{
		std::weibull_distribution<T> dist( a, b );
		return dist( *this );
	}

	/// Produces random numbers according to the extreme value distribution (it is also known as Gumbel Type I, log-Weibull, Fisher-Tippett Type I)
	template <typename T>
	T extreme_value( T a = 0.0, T b = 1.0 )
	{
		std::extreme_value_distribution<T> dist( a, b );
		return dist( *this );
	}

	/// Generates random numbers according to the Normal (or Gaussian) random number distribution
	template <typename T>
	T normal( T mean = 0.0, T stddev = 1.0 )
	{
		std::normal_distribution<T> dist( mean, stddev );
		return dist( *this );
	}

	/// The lognormal_distribution random number distribution produces random numbers x > 0 according to a log-normal distribution
	template <typename T>
	T lognormal( T mean = 0.0, T stddev = 1.0 )
	{
		std::lognormal_distribution<T> dist( mean, stddev );
		return dist( *this );
	}

	/// The chi_squared_distribution produces random numbers x>0 according to the Chi-squared distribution
	template <typename T>
	T chi_squared( T n = 1.0 )
	{
		std::chi_squared_distribution<T> dist( n );
		return dist( *this );
	}

	/// Produces random numbers according to a Cauchy distribution (also called Lorentz distribution)
	template <typename T>
	T cauchy( T a = 0.0, T b = 1.0 )
	{
		std::cauchy_distribution<T> dist( a, b );
		return dist( *this );
	}

	/// Produces random numbers according to the f-distribution
	template <typename T>
	T fisher_f( T m = 1.0, T n = 1.0 )
	{
		std::fisher_f_distribution<T> dist( m, n );
		return dist( *this );
	}

	/// This distribution is used when estimating the mean of an unknown normally distributed value given n+1 independent measurements, each with additive errors of unknown standard deviation, as in physical measurements. Or, alternatively, when estimating the unknown mean of a normal distribution with unknown standard deviation, given n+1 samples
	template <typename T>
	T student_t( T n = 1.0 )
	{
		std::student_t_distribution<T> dist( n );
		return dist( *this );
	}

	/// produces random integers on the interval [0, n), where the probability of each individual integer i is defined as w[i] / S, that is the weight of the ith integer divided by the sum of all n weights
	template <typename T, typename InputIt>
	T discrete( InputIt first, InputIt last )
	{
		std::discrete_distribution<T> dist( first, last );
		return dist( *this );
	}

	/// produces random integers on the interval [0, n), where the probability of each individual integer i is defined as w[i] / S, that is the weight of the ith integer divided by the sum of all n weights
	template <typename T>
	T discrete( std::initializer_list<double> weights )
	{
		std::discrete_distribution<T> dist( weights );
		return dist( *this );
	}

	/// produces random integers on the interval [0, n), where the probability of each individual integer i is defined as w[i] / S, that is the weight of the ith integer divided by the sum of all n weights
	template <typename T, typename UnaryOperation>
	T discrete( std::size_t count, double xmin, double xmax, UnaryOperation unary_op )
	{
		std::discrete_distribution<T> dist( count, xmin, xmax, unary_op );
		return dist( *this );
	}

	/// produces random floating-point numbers, which are uniformly distributed within each of the several subintervals [b[i], b[i] + 1), each with its own weight w[i].The set of interval boundaries and the set of weights are the parameters of this distribution
	template <typename T, typename InputIt1, typename InputIt2>
	T piecewise_constant( InputIt1 first_i, InputIt1 last_i, InputIt2 first_w )
	{
		std::piecewise_constant_distribution<T> dist( first_i, last_i, first_w );
		return dist( *this );
	}

	/// produces random floating-point numbers, which are uniformly distributed within each of the several subintervals [b[i], b[i] + 1), each with its own weight w[i].The set of interval boundaries and the set of weights are the parameters of this distribution
	template <typename T, typename UnaryOperation>
	T piecewise_constant( std::initializer_list<T> bl, UnaryOperation fw )
	{
		std::piecewise_constant_distribution<T> dist( bl, fw );
		return dist( *this );
	}

	/// produces random floating-point numbers, which are uniformly distributed within each of the several subintervals [b[i], b[i] + 1), each with its own weight w[i].The set of interval boundaries and the set of weights are the parameters of this distribution
	template <typename T, typename UnaryOperation>
	T piecewise_constant( std::size_t nw, T xmin, T xmax, UnaryOperation fw )
	{
		std::piecewise_constant_distribution<T> dist( nw, xmin, xmax, fw );
		return dist( *this );
	}

	/// produces random floating-point numbers, which are distributed according to a linear probability density function within each of the several subintervals [b[i], b[i] + 1).The distribution is such that the probability density at each interval boundary is exactly the predefined value p[i]
	template <typename T, typename InputIt1, typename InputIt2>
	T piecewise_linear( InputIt1 first_i, InputIt1 last_i, InputIt2 first_w )
	{
		std::piecewise_linear_distribution<T> dist( first_i, last_i, first_w );
		return dist( *this );
	}

	/// produces random floating-point numbers, which are distributed according to a linear probability density function within each of the several subintervals [b[i], b[i] + 1).The distribution is such that the probability density at each interval boundary is exactly the predefined value p[i]
	template <typename T, typename UnaryOperation>
	T piecewise_linear( std::initializer_list<T> bl, UnaryOperation fw )
	{
		std::piecewise_linear_distribution<T> dist( bl, fw );
		return dist( *this );
	}

	/// produces random floating-point numbers, which are distributed according to a linear probability density function within each of the several subintervals [b[i], b[i] + 1).The distribution is such that the probability density at each interval boundary is exactly the predefined value p[i]
	template <typename T, typename UnaryOperation>
	T piecewise_linear( std::size_t nw, T xmin, T xmax, UnaryOperation fw )
	{
		std::piecewise_linear_distribution<T> dist( nw, xmin, xmax, fw );
		return dist( *this );
	}
};

} // namespace stdx