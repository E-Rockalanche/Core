#ifndef STDX_MATH_HPP
#define STDX_MATH_HPP

#include <cmath>

namespace stdx
{

namespace constants
{

template <typename T>
constexpr T half = static_cast<T>( 5.000000000000000000000000000000000000e-01 );

template <typename T>
constexpr T third = static_cast<T>( 3.333333333333333333333333333333333333e-01 );

template <typename T>
constexpr T two_thirds = static_cast<T>( 6.666666666666666666666666666666666666e-01 );

template <typename T>
constexpr T sqrt_two = static_cast<T>( 1.414213562373095048801688724209698078e+00 );

template <typename T>
constexpr T sqrt_three = static_cast<T>( 1.732050807568877293527446341505872366e+00 );

template <typename T>
constexpr T half_sqrt_two = static_cast<T>( 7.071067811865475244008443621048490392e-01 );

template <typename T>
constexpr T ln_two = static_cast<T>( 6.931471805599453094172321214581765680e-01 );

template <typename T>
constexpr T ln_ln_two = static_cast<T>( -3.665129205816643270124391582326694694e-01 );

template <typename T>
constexpr T sqrt_ln_four = static_cast<T>( 1.177410022515474691011569326459699637e+00 );

template <typename T>
constexpr T inv_sqrt_two = static_cast<T>( 7.071067811865475244008443621048490392e-01 );

template <typename T>
constexpr T pi = static_cast<T>( 3.141592653589793238462643383279502884e+00 );

template <typename T>
constexpr T half_pi = static_cast<T>( 1.570796326794896619231321691639751442e+00 );

template <typename T>
constexpr T third_pi = static_cast<T>( 1.047197551196597746154214461093167628e+00 );

template <typename T>
constexpr T sixth_pi = static_cast<T>( 5.235987755982988730771072305465838140e-01 );

template <typename T>
constexpr T two_pi = static_cast<T>( 6.283185307179586476925286766559005768e+00 );

template <typename T>
constexpr T two_thirds_pi = static_cast<T>( 2.094395102393195492308428922186335256e+00 );

template <typename T>
constexpr T three_quarters_pi = static_cast<T>( 2.356194490192344928846982537459627163e+00 );

template <typename T>
constexpr T four_thirds_pi = static_cast<T>( 4.188790204786390984616857844372670512e+00 );

template <typename T>
constexpr T inv_two_pi = static_cast<T>( 1.591549430918953357688837633725143620e-01 );

template <typename T>
constexpr T sqrt_pi = static_cast<T>( 1.772453850905516027298167483341145182e+00 );

template <typename T>
constexpr T pi_pow_e = static_cast<T>( 2.245915771836104547342715220454373502e+01 );

template <typename T>
constexpr T pi_sqr = static_cast<T>( 9.869604401089358618834490999876151135e+00 );

template <typename T>
constexpr T pi_cubed = static_cast<T>( 3.100627668029982017547631506710139520e+01 );

template <typename T>
constexpr T e = static_cast<T>( 2.718281828459045235360287471352662497e+00 );

template <typename T>
constexpr T e_pow_pi = static_cast<T>( 2.314069263277926900572908636794854738e+01 );

template <typename T>
constexpr T sqrt_e = static_cast<T>( 1.648721270700128146848650787814163571e+00 );

template <typename T>
constexpr T log10_e = static_cast<T>( 4.342944819032518276511289189166050822e-01 );

template <typename T>
constexpr T inv_log10_e = static_cast<T>( 2.302585092994045684017991454684364207e+00 );

template <typename T>
constexpr T ln_ten = static_cast<T>( 2.302585092994045684017991454684364207e+00 );

template <typename T>
constexpr T deg_to_rad = static_cast<T>( 1.745329251994329576923690768488612713e-02 );

template <typename T>
constexpr T rad_to_deg = static_cast<T>( 5.729577951308232087679815481410517033e+01 );

template <typename T>
constexpr T sin_one = static_cast<T>( 8.414709848078965066525023216302989996e-01 );

template <typename T>
constexpr T cos_one = static_cast<T>( 5.403023058681397174009366074429766037e-01 );

template <typename T>
constexpr T sinh_one = static_cast<T>( 1.175201193643801456882381850595600815e+00 );

template <typename T>
constexpr T cosh_one = static_cast<T>( 1.543080634815243778477905620757061682e+00 );

template <typename T>
constexpr T phi = static_cast<T>( 1.618033988749894848204586834365638117e+00 );

template <typename T>
constexpr T ln_phi = static_cast<T>( 4.812118250596034474977589134243684231e-01 );

template <typename T>
constexpr T inv_ln_phi = static_cast<T>( 2.078086921235027537601322606117795767e+00 );

template <typename T>
constexpr T euler = static_cast<T>( 5.772156649015328606065120900824024310e-01 );

template <typename T>
constexpr T inv_euler = static_cast<T>( 1.732454714600633473583025315860829681e+00 );

template <typename T>
constexpr T euler_sqr = static_cast<T>( 3.331779238077186743183761363552442266e-01 );

template <typename T>
constexpr T two_div_pi = static_cast<T>( 6.366197723675813430755350534900574481e-01 );

template <typename T>
constexpr T sqrt_two_div_pi = static_cast<T>( 7.978845608028653558798921198687637369e-01 );

} // namespace constants

template <typename T>
constexpr auto abs( const T& value ) -> std::make_unsigned_t<T>
{
	if constexpr ( std::is_unsigned_v<T> )
		return value;
	else
		return stdx::narrow_cast<std::make_unsigned_t<T>>( ( value < 0 ) ? -value : value );
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr T safe_multiply( T x, T y ) noexcept
{
	const auto result = x * y;
	dbEnsures( x == 0 || result / x == y );
	return stdx::narrow_cast<T>( result );
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr T safe_add( T x, T y ) noexcept
{
	const auto result = x + y;
	dbEnsures( ( x > 0 && y > 0 )
			   ? ( result > 0 )
			   : ( ( x < 0 && y < 0 )
				   ? ( result < 0 )
				   : true ) );
	return result;
}

} // namespace stdx

#endif