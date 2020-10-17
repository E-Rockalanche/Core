#pragma once

#include <UniqueId.h>

#include <stdx/basic_iterator.h>
#include <stdx/memory.h>

#include <vector>

namespace stdx
{

namespace detail
{
template <typename Iterator>
class slot_map_cursor
{
	using it_value_type = typename std::iterator_traits<Iterator>::value_type;

public:
	using key_type = const decltype( std::declval<it_value_type>().first );
	using mapped_type = decltype( std::declval<it_value_type>().second );
	using value_type = std::pair<key_type, mapped_type>;
	using reference = value_type&;
	using pointer = value_type*;

	constexpr slot_map_cursor() noexcept = default;
	constexpr slot_map_cursor( Iterator it, Iterator first, Iterator last ) : m_it{ it }, m_first{ first }, m_last{ last } {}

	constexpr reference read() const noexcept
	{
		return *arrow();
	}

	constexpr pointer arrow() const noexcept
	{
		// make key const
		return reinterpret_cast<pointer>( stdx::to_address( m_it ) );
	}

	constexpr void next() noexcept
	{
		dbExpects( m_it != m_last );
		do
		{
			++m_it;
		}
		while ( m_it != m_last && m_it->first == key_type{} );
	}

	constexpr void prev() noexcept
	{
		dbExpects( m_it != m_first );
		do
		{
			--m_it;
		}
		while ( m_it != m_first && m_it->first == key_type{} );
	}

	constexpr bool equals( const slot_map_cursor& other )
	{
		dbExpects( m_first == other.m_first );
		dbExpects( m_last == other.m_last );
		return m_it == other.m_it;
	}

	std::ptrdiff_t get_index() constr noexcept
	{
		return std::distance( m_first, m_it );
	}

private:
	Iterator m_it{};
	Iterator m_first{};
	Iterator m_last{};
};

} // namespace detail

template <typename T,
	std::size_t IndexBits,
	std::size_t GenerationBits,
	template <class> class Vector = std::vector>
class basic_slot_map
{
public:
	using mapped_type = T;
	using key_type = UniqueId<T, IndexBits, GenerationBits>;

private:
	using imp_value_type = std::pair<key_type, T>;
	using data_type = Vector<imp_value_type>;
	using const_data_type = const Vector<imp_value_type>;

public:
	using value_type = std::pair<const key_type, T>;
	using size_type = typename std::vector<value_type>::size_type;
	using difference_type = typename std::vector<value_type>::difference_type;
	using key_equal = std::equal_to<key_type>;
	using allocator_type = Allocator;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = stdx::basic_iterator<detail::slot_map_cursor<typename data_type::iterator>>;
	using const_iterator = stdx::basic_iterator<detail::slot_map_cursor<typename const_data_type::iterator>>;

	// iterators

	iterator begin() noexcept { return iterator{ m_data.begin(), m_data.begin(), m_data.end() }; }
	iterator end() noexcept { return iterator{ m_data.end(), m_data.begin(), m_data.end() }; }

	const_iterator begin() const noexcept { return const_iterator{ m_data.begin(), m_data.begin(), m_data.end() }; }
	const_iterator end() const noexcept { return const_iterator{ m_data.end(), m_data.begin(), m_data.end() }; }

	const_iterator cbegin() const noexcept { return begin() }
	const_iterator cend() const noexcept { return end(); }

	// capacity

	bool empty() const noexcept
	{
		return size() == 0;
	}

	size_type size() const noexcept
	{
		return m_data.size() - m_freeKeys.size();
	}

	size_type max_size() const noexcept
	{
		return m_data.max_size();
	}

	// modifiers

	void clear() noexcept
	{
		m_data.clear();
		m_freeKeys.clear();
	}

	key_type insert( const T& value )
	{
		return emplace( value );
	}

	key_type insert( T&& value )
	{
		return emplace( std::move( value ) );
	}

	template <typename... Args>
	key_type emplace( Args&&... args )
	{
		if ( m_freeKeys.empty() )
		{
			key_type key{ m_data.size(), 0 };
			m_data.push_back( { key, T{ std::forward<Args>( args )... } } );
			return key;
		}
		else
		{
			key_type oldKey = m_freeKeys.back();
			m_freeKeys.pop_back();
			key_type key{ oldKey.GetIndex(), oldKey.GetGeneration() + 1 };
			m_data[ key.GetIndex() ] = { key, T{ std::forward<Args>( args )... } };
			return key;
		}
	}

	iterator erase( const_iterator pos )
	{
		dbAssert( std::distance( cbegin(), pos ) < size() );
		auto it = m_data.begin() + pos.cursor().get_index();
		erase_imp( *it );
		return iterator{ ++it, m_data.begin(), m_data.end() };
	}

	size_type erase( key_type key )
	{
		auto* entry = find_imp( key );
		if ( entry )
		{
			erase_imp( *entry );
			return 1;
		}
		return 0;
	}

	void swap( slot_map& other ) noexcept
	{
		auto temp = std::move( *this );
		*this = std::move( other );
		other = std::move( temp );
	}

	T& operator[]( key_type key ) noexcept
	{
		dbAssert( key.GetIndex() < m_data.size() );
		auto& value = m_data[ key.GetIndex() ];
		dbAssert( value.first == key );
		return value.second;
	}

	const T& operator[]( key_type key ) const noexcept
	{
		dbAssert( key.GetIndex() < m_data.size() );
		auto& value = m_data[ key.GetIndex() ];
		dbAssert( value.first == key );
		return value.second;
	}

	bool contains( key_type key ) const noexcept
	{
		return ( key.GetIndex() < m_data.size() ) && ( m_data[ key.GetIndex() ].first == key );
	}

	size_type count( key_type key ) const noexcept
	{
		return static_cast<size_type>( contains() );
	}

	iterator find( key_type key ) noexcept
	{
		if ( contains() )
			return iterator{ m_data.begin() + key.GetIndex(), m_data.begin(), m_data.end() };

		return end();
	}

	const_iterator find( key_type key ) const noexcept
	{
		if ( contains() )
			return const_iterator{ m_data.begin() + key.GetIndex(), m_data.begin(), m_data.end() };

		return end();
	}

private:
	void erase_imp( value_type& value )
	{
		m_freeKeys.push_back( value.first );
		value = { key_type{}, T{} };
	}

private:
	Vector<imp_value_type> m_data;
	Vector<key_type> m_freeKeys;
};

template <typename T>
using slot_map = basic_slot_map<T, 32, 32, std::vector>;

} // namespace stdx