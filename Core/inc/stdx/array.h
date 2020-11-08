#pragma once

#include <array>
#include <type_traits>

namespace stdx
{

// make_array

namespace details
{

	template<class> struct is_ref_wrapper : std::false_type {};
	template<class T> struct is_ref_wrapper<std::reference_wrapper<T>> : std::true_type {};

	template<class T>
	using not_ref_wrapper = std::negation<is_ref_wrapper<std::decay_t<T>>>;

	template <class D, class...> struct return_type_helper { using type = D; };
	template <class... Types>
	struct return_type_helper<void, Types...> : std::common_type<Types...> {
		static_assert( std::conjunction_v<not_ref_wrapper<Types>...>,
			"Types cannot contain reference_wrappers when D is void" );
	};

	template <class D, class... Types>
	using return_type = std::array<typename return_type_helper<D, Types...>::type,
		sizeof...( Types )>;

} // namespace detail

template <class D = void, class... Types>
constexpr details::return_type<D, Types...> make_array( Types&&... t ) {
	return { std::forward<Types>( t )... };
}

// to_array

namespace detail
{

	template <class T, std::size_t N, std::size_t... I>
	constexpr std::array<std::remove_cv_t<T>, N>
		to_array_impl( T( &a )[ N ], std::index_sequence<I...> )
	{
		return { { a[ I ]... } };
	}

	template <class T, std::size_t N, std::size_t... I>
	constexpr std::array<std::remove_cv_t<T>, N>
		to_array_impl( T( &&a )[ N ], std::index_sequence<I...> )
	{
		return { { std::move( a[ I ] )... } };
	}

}

template <class T, std::size_t N>
constexpr std::array<std::remove_cv_t<T>, N> to_array( T( &a )[ N ] )
{
	return detail::to_array_impl( a, std::make_index_sequence<N>{} );
}

template <class T, std::size_t N>
constexpr std::array<std::remove_cv_t<T>, N> to_array( T( &&a )[ N ] )
{
	return detail::to_array_impl( std::move( a ), std::make_index_sequence<N>{} );
}

} // namespace stdx