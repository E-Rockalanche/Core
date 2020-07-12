#pragma once

#include <stdx/span.h>

namespace stdx
{

constexpr std::ptrdiff_t dynamic_extent = -1;

template <std::ptrdiff_t... StaticExtents>
class extents;

template<ptrdiff_t ... Extents1, ptrdiff_t ... Extents2>
constexpr bool operator==( extents<Extents1...> const & lhs, extents<Extents2...> const & rhs )
{
	return ( ( Extents1 == Extents2 ) && ... );
}

template<ptrdiff_t ... Extents1, ptrdiff_t ... Extents2>
constexpr bool operator!=( extents<Extents1...> const & lhs, extents<Extents2...> const & rhs )
{
	return ( ( Extents1 != Extents2 ) || ... );
}

class layout_right;
class layout_left;
class layout_stride;

class accessor_basic;

template <typename T, typename Extents, typename LayoutPolicy = layout_right, typename AccessorPolicy = accessor_basic>
class basic_mdspan
{
public:
	using layout = LayoutPolicy;
	using mapping = typename layout::mapping<Extents>;
	using accessor = typename AccessorPolicy::accesor<T>;

	using element_type = typename accessor::value_type;
	using value_type = std::remove_cv_t<element_type>;
	using index_type = std::ptrdiff_t;
	using difference_type = std::ptrdiff_t;
	using pointer = typename accessor::pointer;
	using reference = typename accessor::reference;

	constexpr basic_mdspan() noexcept;

	constexpr basic_mdspan( basic_mdspan&& ) noexcept = default;

	constexpr basic_mdspan( basic_mdspan const& ) noexcept = default;

	constexpr basic_mdspan& operator=( basic_mdspan&& ) noexcept = default;

	constexpr basic_mdspan& operator=( basic_mdspan const& ) noexcept = default;

	template <typename... IndexType>
	explicit constexpr basic_mdspan( pointer data, IndexType... dynamicExtents ) noexcept;

};

template <typename T, std::ptrdiff_t... Extents>
using mdspan = basic_mdspan<T, extents<Extents>>;

}