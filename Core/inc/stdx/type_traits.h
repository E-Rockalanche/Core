#ifndef STDX_TYPE_TRAITS_HPP
#define STDX_TYPE_TRAITS_HPP

#include <iterator>
#include <type_traits>
#include <utility>

namespace stdx {

// detects if a type is integral, unsigned, and not boolean
template <typename T>
struct is_unsigned_integral
{
	static constexpr bool value = std::is_unsigned_v<T> && std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;
};

template <typename T>
inline constexpr bool is_unsigned_integral_v = is_unsigned_integral<T>::value;

// is integral or floating point, but not boolean

template <typename T>
struct is_numeric
{
	static constexpr bool value = ( std::is_integral_v<T> || std::is_floating_point_v<T> ) && !std::is_same_v<std::remove_cv_t<T>, bool>;
};

template <typename T>
inline constexpr bool is_numeric_v = is_numeric<T>::value;

// check if type exists in parameter pack

template <typename Type, typename... Types>
struct is_present
{
	static constexpr bool value = ( std::is_same_v<Type, Types> || ... );
};

template <typename Type, typename... Types>
inline constexpr bool is_present_v = is_present<Type, Types...>::value;

// get index of type in parameter pack

template <typename T, typename... Ts>
struct index_of;

template <typename T, typename... Ts>
struct index_of<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct index_of<T, U, Ts...> : std::integral_constant<std::size_t, 1 + index_of<T, Ts...>::value> {};

template <typename T, typename... Ts>
inline constexpr std::size_t index_of_v = index_of<T, Ts...>::value;

// use constness of one type on another

template <typename Of, typename On>
struct use_constness
{
	using type = typename std::conditional<std::is_const_v<Of>, std::add_const_t<On>, On>::type;
};

template <typename Of, typename On>
using use_constness_t = typename use_constness<Of, On>::type;

template <typename T>
struct has_no_extents
{
	static constexpr bool value = std::is_same_v<std::remove_all_extents_t<T>, T>;
};

template <typename T>
inline constexpr bool has_no_extents_v = has_no_extents<T>::value;

// detection toolkit
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4502.pdf

namespace detail
{
	// nonesuch represents no type. Used for detecting type trait failures
	struct nonesuch
	{
		nonesuch() = delete;
		nonesuch( const nonesuch& ) = delete;
		~nonesuch() = delete;
		void operator=( const nonesuch& ) = delete;
	};

	template <class Default,
		class, // always void, supplied externally
		template<class...> class Op,
		class... Args>
	struct detector
	{
		using value_t = typename std::false_type;
		using type = Default;
	};

	template <class Default, template<class...> class Op, class... Args>
	struct detector<Default, std::void_t<Op<Args...>>, Op, Args...>
	{
		using value_t = typename std::true_type;
		using type = Op<Args...>;
	};
}

// determines if Op<Args...> is valid
template <template<class...> class Op, class... Args>
using is_detected = typename detail::detector<detail::nonesuch, void, Op, Args...>::value_t;

template <template<class...> class Op, class... Args>
inline constexpr bool is_detected_v = is_detected<Op, Args...>::value;

// returns type of Op<Args...> if it exists
template <template<class...> class Op, class... Args>
using detected_t = typename detail::detector<detail::nonesuch, void, Op, Args...>::type;

// returns type of Op<Args...> if it exists, Default otherwise
template <class Default, template<class...> class Op, class... Args>
using detected_or = typename detail::detector<Default, void, Op, Args...>;

template <class Default, template<class...> class Op, class... Args>
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

// returns if Op<Args...> is the same type as Expected
template <class Expected, template<class...> class Op, class... Args>
using is_detected_exact = typename std::is_same<Expected, detected_t<Op, Args...>>;

template <class Expected, template<class...> class Op, class... Args>
inline constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

// returns if Op<Args...> is convertible to To
template <class To, template<class...> class Op, class... Args>
using is_detected_convertible = typename std::is_convertible<detected_t<Op, Args...>, To>;

template <class To, template<class...> class Op, class... Args>
inline constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;



// function traits

template <typename>
struct function_traits;

template <typename Function>
struct function_traits : public function_traits<decltype( &Function::operator() )> {};

template <typename ClassType, typename ReturnType, typename... Arguments>
struct function_traits<ReturnType( ClassType::* )( Arguments... ) const>
{
	using return_type = ReturnType;

	using arguments_tuple = std::tuple<Arguments...>;

	template <std::size_t I>
	using argument_type = typename std::tuple_element<I, arguments_tuple>;

	static constexpr std::size_t arity = sizeof...( Arguments );
};

namespace detail
{
	template <typename T, typename... Args>
	using function_t = decltype( std::declval<T>()( std::declval<Args>()... ) );
}

template <typename T, typename Ret, typename... Args>
inline constexpr bool has_signature_v = std::is_same_v<detected_t<detail::function_t, T, Args...>, Ret>;


// container concepts

namespace detail
{
	template <typename T>
	using push_back_t = decltype( std::declval<T>().push_back( std::declval<typename T::value_type>() ) );

	template <typename T>
	using pop_back_t = decltype( std::declval<T>().pop_back() );

	template <typename T>
	using insert_t = decltype( std::declval<T>().insert( std::declval<typename T::value_type>() ) );

	template <typename T>
	using find_t = decltype( std::declval<T>().find( std::declval<typename T::key_type>() ) );

	template <typename T>
	using begin_t = decltype( std::begin( std::declval<T>() ) );

	template <typename T>
	using end_t = decltype( std::end( std::declval<T>() ) );

	template <typename T>
	using static_size_t = std::enable_if_t<std::size( std::declval<T>() ) >= 0, void>;

	template <typename T>
	using index_t = decltype( std::declval<T>()[ std::declval<std::size_t>() ] );

	template <typename T>
	using key_type_t = typename T::key_type;

	template <typename T>
	using mapped_type_t = typename T::mapped_type;

	template <typename T>
	using arrow_operator_t = decltype( std::declval<T>().operator->() );

	template <typename T>
	using deref_operator_t = decltype( std::declval<T>().operator*() );
}

template <typename T>
inline constexpr bool is_iterable_v = is_detected_v<detail::begin_t, T> && is_detected_v<detail::end_t, T>;

template <typename T>
inline constexpr bool is_map_v =
	is_detected_v<detail::insert_t, T> &&
	is_detected_v<detail::find_t, T> &&
	is_detected_v<detail::key_type_t, T> &&
	is_detected_v<detail::mapped_type_t, T> &&
	is_iterable_v<T>;

template <typename T>
inline constexpr bool is_set_v =
	is_detected_v<detail::insert_t, T> &&
	is_detected_v<detail::find_t, T> &&
	is_detected_v<detail::key_type_t, T> &&
	!is_detected_v<detail::mapped_type_t, T> &&
	is_iterable_v<T>;

template <typename T>
inline constexpr bool is_list_v = is_detected_v<detail::push_back_t, T> && is_detected_v<detail::pop_back_t, T> && is_iterable_v<T>;

template <typename T>
inline constexpr bool is_array_like_v = is_detected_v<detail::index_t, T> && is_detected_v<detail::static_size_t, T> && is_iterable_v<T>;

template <typename T>
inline constexpr bool is_pointer_like_v =
	std::is_pointer<T> ||
	( is_detected_v<detail::arrow_operator_t, T> && is_detected_v<detail::deref_operator_t, T> );

} // namespace stdx

#endif // STDX_TYPE_TRAITS_HPP