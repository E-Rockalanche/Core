#pragma once

#include <stdx/compressed_pair.h>

#include <algorithm>
#include <utility>
#include <vector>

namespace stdx
{

template <typename Key, typename T, typename Compare = std::less<Key>>
class flat_map
{
	using storage_value_type = std::pair<Key, T>;
	using storage_type = std::vector<storage_value_type>;

public:
	using key_type = Key;
	using mapped_type = T;
	using value_type = std::pair<const key_type, mapped_type>;

	using size_type = typename storage_type::size_type;
	using difference_type = typename storage_type::difference_type;

	using key_compare = Compare;

	using reference = value_type&;
	using const_reference = const value_type&;

	using pointer = value_type*;
	using const_pointer = const value_type*;

	using iterator = typename storage_type::iterator;
	using const_iterator = typename storage_type::const_iterator;
	using reverse_iterator = typename storage_type::reverse_iterator;
	using const_reverse_iterator = typename storage_type::const_reverse_iterator;

	flat_map() noexcept( noexcept( storage_type() ) ) = default;

	explicit flat_map( const Compare& comp ) noexcept : m_compare( comp ) {}

	template <typename InputIt>
	flat_map( InputIt first, InputIt last, const Compare& comp = Compare() )
		: m_compare( comp )
	{
		get_storage().reserve( static_cast<size_type>( std::distance( first, last ) ) );
		std::foreach( first, last, [this]( auto& value ) { insert( value ); } );
	}

	flat_map( const flat_map& ) = default;

	flat_map( flat_map&& ) noexcept = default;

	flat_map( std::initializer_list<value_type> init, const Compare& comp = Compare() ) : flat_map( init.begin(), init.end(), comp ) {}

	~flat_map() = default;

	flat_map& operator=( const flat_map& ) = default;

	flat_map& operator=( flat_map&& ) noexcept = default;

	flat_map& operator=( std::initializer_list<value_type> init )
	{
		return *this = flat_map( init );
	}

	// element access

	T& at( const Key& key )
	{
		auto it = find( key );
		if ( it == end() )
			throw std::out_of_range();

		return it->second;
	}

	const T& at( const Key& key ) const
	{
		auto it = find( key );
		if ( it == end() )
			throw std::out_of_range();

		return it->second;
	}

	T& operator[]( const Key& key )
	{
		auto it = lower_bound( key );
		if ( it == end() || get_compare()( key, it->first ) )
			it = get_storage().insert( it, { key, mapped_type() } );

		return it->second;
	}

	T& operator[]( Key&& key )
	{
		auto it = lower_bound( key );
		if ( it == end() || get_compare()( key, it->first ) )
			it = get_storage().insert( it, { std::move( key ), mapped_type() } );

		return it->second;
	}

	pointer data() noexcept
	{
		return reinterpret_cast<pointer>( get_storage().data() );
	}

	const_pointer data() const noexcept
	{
		return reinterpret_cast<const_pointer>( get_storage().data() );
	}

	// iterators

	iterator begin() noexcept { return get_storage().begin(); }
	iterator end() noexcept { return get_storage().end(); }

	const_iterator begin() const noexcept { return get_storage().begin(); }
	const_iterator end() const noexcept { return get_storage().end(); }

	const_iterator cbegin() const noexcept { return get_storage().begin(); }
	const_iterator cend() const noexcept { return get_storage().end(); }

	reverse_iterator rbegin() noexcept { return get_storage().rbegin(); }
	reverse_iterator rend() noexcept { return get_storage().rend(); }

	const_reverse_iterator rbegin() const noexcept { return get_storage().rbegin(); }
	const_reverse_iterator rend() const noexcept { return get_storage().rend(); }

	const_reverse_iterator crbegin() const noexcept { return get_storage().rbegin(); }
	const_reverse_iterator crend() const noexcept { return get_storage().rend(); }

	// capacity

	[[nodiscard]] bool empty() const noexcept { return get_storage().empty(); }
	size_type size() const noexcept { return get_storage().size(); }
	size_type max_size() const noexcept { return get_storage().max_size(); }
	void reserve( size_type n ) { get_storage().reserve( n ); }
	size_type capacity() const noexcept { return get_storage().capacity(); }
	void shrink_to_fit() { get_storage().shrink_to_fit(); }

	// modifiers

	void clear()
	{
		get_storage().clear();
	}

	std::pair<iterator, bool> insert( const value_type& value )
	{
		auto it = lower_bound( value.first );
		if ( it == end() || get_compare()( value.first, it->first ) )
		{
			it = get_storage().insert( it, value );
			return { it, true };
		}
		else
		{
			return { it, false };
		}
	}

	std::pair<iterator, bool> insert( value_type&& value )
	{
		auto it = lower_bound( value.first );
		if ( it == end() || get_compare()( value.first, it->first ) )
		{
			it = get_storage().insert( it, std::move( value ) );
			return { it, true };
		}
		else
		{
			return { it, false };
		}
	}

	template <typename InputIt>
	void insert( InputIt first, InputIt last )
	{
		std::foreach( first, last, [this]( auto& value ) { insert( value ); } );
	}

	void insert( std::initializer_list<value_type> init )
	{
		insert( init.begin(), init.end() );
	}

	template <typename M>
	std::pair<iterator, bool> insert_or_assign( const Key& k, M&& obj )
	{
		auto it = lower_bound( k );
		if ( it == end() || get_compare()( k, it->first ) )
		{
			it = get_storage().insert( it, { k, std::forward<M>( obj ) } );
			return { it, true };
		}
		else
		{
			it->second = std::forward<M>( obj );
			return { it, false };
		}
	}

	template <typename M>
	std::pair<iterator, bool> insert_or_assign( Key&& k, M&& obj )
	{
		auto it = lower_bound( k );
		if ( it == end() || get_compare()( k, it->first ) )
		{
			it = get_storage().insert( it, { std::move( k ), std::forward<M>( obj ) } );
			return { it, true };
		}
		else
		{
			it->second = std::forward<M>( obj );
			return { it, false };
		}
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace( Args&&... args )
	{
		return insert( storage_value_type( std::forward<Args>( args )... ) );
	}

	template <typename... Args>
	std::pair<iterator, bool> try_emplace( const Key& k, Args&&... args )
	{
		auto it = lower_bound( k );
		if ( it == end() || get_compare()( k, it->first ) )
		{
			it = get_storage().insert( it, { k, mapped_type( std::forward<Args>( args )... ) } );
			return { it, true };
		}
		else
		{
			return { it, false };
		}
	}

	template <typename... Args>
	std::pair<iterator, bool> try_emplace( Key&& k, Args&&... args )
	{
		auto it = lower_bound( k );
		if ( it == end() || get_compare()( k, it->first ) )
		{
			it = get_storage().insert( it, { std::move( k ), mapped_type( std::forward<Args>( args )... ) } );
			return { it, true };
		}
		else
		{
			return { it, false };
		}
	}

	iterator erase( const_iterator pos )
	{
		return get_storage().erase( pos );
	}

	iterator erase( const_iterator first, const_iterator last )
	{
		return get_storage().erase( first, last );
	}

	size_type erase( const Key& k )
	{
		auto it = find( k );
		if ( it != end() )
		{
			get_storage().erase( it );
			return 1;
		}
		else
		{
			return 0;
		}
	}

	void swap( flat_map& other ) noexcept
	{
		std::swap( m_storage, other.m_storage );
		std::swap( m_compare, other.m_compare );
	}

	// lookup

	template <typename K>
	size_type count( const K& k ) const
	{
		return static_cast<size_type>( find( k ) != end() );
	}

	template <typename K>
	iterator find( const K& k )
	{
		auto it = lower_bound( k );
		return ( it == end() || get_compare()( k, it->first ) ) ? end() : it;
	}

	template <typename K>
	const_iterator find( const K& k ) const
	{
		auto it = lower_bound( k );
		return ( it == end() || get_compare()( k, it->first ) ) ? end() : it;
	}

	template <typename K>
	bool contains( const K& k ) const
	{
		return find( k ) != end();
	}

	template <typename K>
	std::pair<iterator, iterator> equal_range( const K& k )
	{
		return std::equal_range( begin(), end(), k, two_way_compare{ get_compare() } );
	}

	template <typename K>
	std::pair<const_iterator, const_iterator> equal_range( const K& k ) const
	{
		return std::equal_range( begin(), end(), k, two_way_compare{ get_compare() } );
	}

	template <typename K>
	iterator lower_bound( const K& k )
	{
		return std::lower_bound( begin(), end(), k, two_way_compare{ get_compare() } );
	}

	template <typename K>
	const_iterator lower_bound( const K& k ) const
	{
		return std::lower_bound( begin(), end(), k, two_way_compare{ get_compare() } );
	}

	template <typename K>
	iterator upper_bound( const K& k )
	{
		return std::upper_bound( begin(), end(), k, two_way_compare{ get_compare() } );
	}

	template <typename K>
	const_iterator upper_bound( const K& k ) const
	{
		return std::upper_bound( begin(), end(), k, two_way_compare{ get_compare() } );
	}

	// observers

	key_compare key_comp() const
	{
		return get_compare();
	}

	// non-member functions

	friend bool operator==( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.get_storage() == rhs.get_storage();
	}

	friend bool operator!=( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.get_storage() != rhs.get_storage();
	}

	friend bool operator<( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.get_storage() < rhs.get_storage();
	}

	friend bool operator>( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.get_storage() > rhs.get_storage();
	}

	friend bool operator<=( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.get_storage() <= rhs.get_storage();
	}

	friend bool operator>=( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.get_storage() >= rhs.get_storage();
	}

private:

	struct two_way_compare
	{
		const key_compare& comp;

		template <typename K>
		bool operator()( const storage_value_type& value, const K& k )
		{
			return comp( value.first, k );
		}

		template <typename K>
		bool operator()( const K& k, const storage_value_type& value )
		{
			return comp( k, value.first );
		}
	};

private:
	auto& get_storage() noexcept { return m_storage; }
	auto& get_storage() const noexcept { return m_storage; }
	auto& get_compare() const noexcept { return m_compare; }

private:
	// TODO: wrap in compressed pair
	storage_type m_storage;
	key_compare m_compare;
};

} // namespace stdx
