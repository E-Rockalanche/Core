#pragma once

#include <stdx/assert.h>
#include <stdx/iterator/basic_iterator.h>

#include <array>

namespace stdx
{

namespace detail
{

	template<typename StaticMap, typename T>
	class static_map_cursor
	{
	public:
		using key_type = typename StaticMap::key_type;
		using mapped_type = T;
		using size_type = typename StaticMap::size_type;
		using difference_type = typename StaticMap::difference_type;
		using reference = std::pair<const key_type, T&>;

		constexpr static_map_cursor() noexcept = default;

		constexpr static_map_cursor( const static_map_cursor& ) noexcept = default;

		constexpr static_map_cursor( StaticMap* map, difference_type index ) : m_map{ map }, m_index{ index } {}

		template <typename Map2, typename T2>
		constexpr static_map_cursor( const static_map_cursor<Map2, T2>& other ) noexcept : m_map{ other.m_map }, m_index{ other.m_index } {}

		constexpr static_map_cursor& operator=( const static_map_cursor& ) noexcept = default;

		template <typename Map2, typename T2>
		constexpr static_map_cursor& operator=( const static_map_cursor<Map2, T2>& other ) noexcept
		{
			m_map = other.m_map;
			m_index = other.m_index;
			return *this;
		}

		constexpr reference read() const noexcept
		{
			dbExpects( m_map );
			return reference{ m_map->keys()[ m_index ], m_map->values()[ m_index ] };
		}

		constexpr void next() noexcept
		{
			dbExpects( m_map );
			++m_index;
			dbEnsures( m_index < m_map->ssize() );
		}

		constexpr void prev() noexcept
		{
			dbExpects( m_map );
			dbExpects( m_index > 0 );
			--m_index;
		}

		constexpr void advance( difference_type n ) noexcept
		{
			dbExpects( m_map );
			m_index += n;
			dbEnsures( 0 <= m_index && m_index < m_map->ssize() );
		}

		constexpr bool equal( const static_map_cursor& other ) const noexcept
		{
			dbExpects( m_map && m_map == other.m_map );
			return m_index == other.m_index;
		}

		constexpr difference_type distance_to( const static_map_cursor& other ) const noexcept
		{
			dbExpects( m_map && m_map == other.m_map );
			return other.m_index - m_index;
		}

	private:
		StaticMap* m_map = nullptr;
		difference_type m_index = 0;

		template <typename Map2, typename T2>
		friend class static_map_cursor;
	};

}

template<typename T, auto... Keys>
class static_map
{
	using storage_type = std::array<T, sizeof...( Keys )>;

public:
	using key_type = std::common_type_t<decltype( Keys )...>;
	using mapped_type = T;
	using value_type = std::pair<const key_type, T>;
	using size_type = typename storage_type::size_type;
	using difference_type = typename storage_type::difference_type;
	using reference = std::pair<const key_type, T&>;
	using const_reference = std::pair<const key_type, const T&>;

	using iterator = basic_iterator<detail::static_map_cursor<static_map<T, Keys...>, T>>;
	using const_iterator = basic_iterator<detail::static_map_cursor<const static_map<T, Keys...>, const T>>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	// construction/assignment

	constexpr static_map() = default;

	// initialize all keys
	constexpr static_map( std::initializer_list<value_type> init )
	{
#ifdef _DEBUG
		std::array<bool, sizeof...( Keys )> initialized{};
		int initCount = 0;
#endif

		for ( auto& value : init )
		{
			const auto index = get_key_index( value.first );
			m_data[ index ] = value.second;

#ifdef _DEBUG
			dbAssert( !initialized[ index ] );
			initialized[ index ] = true;
			++initCount;
#endif
		}

#ifdef _DEBUG
		dbAssert( initCount == sizeof...( Keys ) );
#endif
	}

	constexpr static_map( const static_map& ) = default;

	constexpr static_map( static_map&& ) noexcept = default;

	constexpr static_map& operator=( std::initializer_list<value_type> init )
	{
		return *this = static_map{ init };
	}

	// iterators

	constexpr iterator begin() noexcept { return iterator( { *this, 0 } ); }
	constexpr iterator end() noexcept { return iterator( { *this, ssize() } ); }
	constexpr const_iterator begin() const noexcept { return const_iterator( { *this, 0 } ); }
	constexpr const_iterator end() const noexcept { return const_iterator( { *this, ssize() } ); }
	constexpr const_iterator cbegin() const noexcept { return begin(); }
	constexpr const_iterator cend() const noexcept { return end(); }
	constexpr reverse_iterator rbegin() noexcept { return reverse_iterator( end() ); }
	constexpr reverse_iterator rend() noexcept { return reverse_iterator( begin() ); }
	constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator( end() ); }
	constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator( begin() ); }
	constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator( end() ); }
	constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator( begin() ); }

	// capacity

	constexpr bool empty() const noexcept
	{
		return m_data.empty();
	}

	constexpr size_type size() const noexcept
	{
		return m_data.size();
	}

	constexpr difference_type ssize() const noexcept
	{
		return static_cast<difference_type>( m_data.size() );
	}

	constexpr size_type max_size() const noexcept
	{
		return m_data.size();
	}

	// access

	constexpr const auto& keys() const noexcept
	{
		static const std::array<key_type, sizeof...( Keys )> s_keys{ Keys... };
		return s_keys;
	}

	constexpr auto& values() noexcept
	{
		return m_data;
	}

	constexpr const auto& values() const noexcept
	{
		return m_data;
	}

	constexpr T& at( const key_type& key )
	{
		const auto index = get_key_index( key );
		if ( index >= m_data.size() )
			throw std::out_of_range();

		return m_data[ index ];
	}

	constexpr const T& at( const key_type& key ) const
	{
		const auto index = get_key_index( key );
		if ( index >= m_data.size() )
			throw std::out_of_range();

		return m_data[ index ];
	}

	constexpr T& operator[]( const key_type& key ) noexcept
	{
		const auto index = get_key_index( key );
		dbExpects( index < size() );
		return m_data[ index ];
	}

	constexpr const T& operator[]( const key_type& key ) const noexcept
	{
		const auto index = get_key_index( key );
		dbExpects( index < size() );
		return m_data[ index ];
	}

	// operations

	constexpr void swap( static_map& other )
	{
		m_data.swap( other.m_data );
	}

	constexpr size_type contains( const key_type& key ) const noexcept
	{
		return get_key_index( key ) < sizeof...( Keys );
	}

	constexpr size_type count( const key_type& key ) const noexcept
	{
		return contains( key ) ? 1 : 0;
	}

	constexpr void fill( const T& value )
	{
		m_data.fill( value );
	}

	iterator find( const key_type& key )
	{
		const auto index = get_key_index( key );
		return ( index < size() ) ? iterator{ this, index } : end();
	}

	const_iterator find( const key_type& key ) const
	{
		const auto index = get_key_index( key );
		return ( index < size() ) ? const_iterator{ this, index } : end();
	}

private:
	static constexpr size_type get_key_index( key_type key ) noexcept
	{
		auto& my_keys = keys();
		for ( size_type i = 0; i < my_keys.size(); ++i )
		{
			if ( my_keys[ i ] == key )
				return i;
		}
		return std::numeric_limits<size_type>::max();
	}

private:
	storage_type m_data;
};

}