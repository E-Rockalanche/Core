#pragma once

#include <stdx/assert.h>
#include <stdx/iterator/basic_iterator.h>

namespace stdx
{

namespace detail
{

template <typename Iterator>
using filter_equal_t = decltype( std::declval<Iterator>() == std::declval<Iterator>() );

template <typename Iterator, typename UnaryPred>
class filter_iterator_cursor
{
public:
	constexpr filter_iterator_cursor( Iterator it, Iterator last, UnaryPred p ) noexcept
		: m_it( std::move( it ) )
		, m_last( std::move( last ) )
		, m_predicate( std::move( p ) )
	{}

	constexpr auto& read() const noexcept
	{
		dbExpects( m_it != m_last );
		return *m_it;
	}

	constexpr void next() noexcept
	{
		dbExpects( m_it != m_last );
		do
		{
			++m_it;
		}
		while ( m_it != m_last && !m_predicate( *m_it ) );
	}

	constexpr bool equal( const filter_iterator_cursor& other ) const noexcept
	{
		dbExpects( m_last == other.m_last );
		return m_it == other.m_it;
	}

private:
	Iterator m_it;
	Iterator m_last;
	UnaryPred m_predicate;
};

} // namespace detail

template <typename Iterator, typename UnaryPred>
class filter_iterator : public stdx::basic_iterator<detail::filter_iterator_cursor<Iterator, UnaryPred>>
{
public:
	using stdx::basic_iterator<detail::filter_iterator_cursor<Iterator, UnaryPred>>::basic_iterator;
};

} // namespace stdx