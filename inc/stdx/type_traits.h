#ifndef STDX_TYPE_TRAITS_HPP
#define STDX_TYPE_TRAITS_HPP

#include <type_traits>

namespace stdx {

// detects if a type is integral, unsigned, and not bool
template <typename T>
struct is_unsigned_integral
{
	static constexpr bool value = std::is_unsigned<T>::value && std::is_integral<T>::value && !std::is_same<T, bool>::value;
};

template <typename T>
constexpr bool is_unsigned_integral_v = is_unsigned_integral<T>::value;

template <typename Type, typename... Types>
struct is_present
{
	static constexpr bool value = ( std::is_same_v<Type, Types> || ... );
};

template <typename Type, typename... Types>
constexpr bool is_present_v = is_present<Type, Types...>::value;

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
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

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
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

// returns if Op<Args...> is convertible to To
template <class To, template<class...> class Op, class... Args>
using is_detected_convertible = typename std::is_convertible<detected_t<Op, Args...>, To>;

template <class To, template<class...> class Op, class... Args>
constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;

} // namespace stdx

#endif // STDX_TYPE_TRAITS_HPP