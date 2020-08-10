#ifndef STDX_FLAT_MAP_HPP
#define STDX_FLAT_MAP_HPP

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace stdx
{

template <class Key, class T, class Compare = std::less<Key>>
class flat_map
{
	using value_type_imp = std::pair<Key, T>;

public:
	using key_type = Key;
	using mapped_type = T;
	using value_type = std::pair<const Key, T>;
	using reference = value_type &;
	using const_reference = const value_type&;
	using pointer = value_type *;
	using const_pointer = const value_type*;

	using size_type = typename std::vector<value_type_imp>::size_type;
	using difference_type = typename std::vector<value_type_imp>::difference_type;

	using iterator = typename std::vector<value_type_imp>::iterator;
	using const_iterator = typename std::vector<value_type_imp>::const_iterator;
	using reverse_iterator = typename std::vector<value_type_imp>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<value_type_imp>::const_reverse_iterator;

	using key_compare = Compare;

public:

	// construction/assignment

	flat_map() noexcept = default;

	template <class InputIt>
	flat_map( InputIt first, InputIt last )
	{
		m_values.reserve( static_cast<size_t>( last - first ) );
		for(auto it = first; it != last; ++it)
			dbVerify( insert( *it ).second );
	}

	flat_map( const flat_map& ) = default;

	flat_map( flat_map&& ) noexcept = default;

	flat_map( std::initializer_list<value_type> init ) : flat_map( init.begin(), init.end() ) {}

	~flat_map() = default;

	flat_map& operator=( const flat_map& other ) = default;

	flat_map& operator=( flat_map&& other ) noexcept = default;

	flat_map& operator=( std::initializer_list<value_type> init )
	{
		clear();
		reserve( init.size() );
		for( auto& value : init )
			dbVerify( insert( value ).second );
	}

	// element access

	T& at( const Key& key )
	{
		auto it = lower_bound( key );
		if ( it->first != key )
			throw std::out_of_range( OUT_OF_RANGE_MESSAGE );
		return *it;
	}

	const T& at( const Key& key ) const
	{
		auto it = lower_bound( key );
		if ( it->first != key )
			throw std::out_of_range( OUT_OF_RANGE_MESSAGE );
		return *it;
	}

	T& operator[]( const Key& key )
	{
		const auto it = lower_bound( key );
		if ( isEntry( it, key ) )
			return it->second;

		return emplace_hint( it, key, T{} )->second;
	}

	T& operator[]( Key&& key )
	{
		const auto it = lower_bound( key );
		if ( isEntry( it, key ) )
			return it->second;

		return emplace_hint( it, std::move( key ), T{} )->second;
	}

	// iterators

	iterator begin() noexcept { return m_values.begin(); }
	iterator end() noexcept { return m_values.end(); }

	const_iterator begin() const noexcept { return m_values.cbegin(); }
	const_iterator end() const noexcept { return m_values.cend(); }

	const_iterator cbegin() const noexcept { return m_values.cbegin(); }
	const_iterator cend() const noexcept { return m_values.cend(); }

	reverse_iterator rbegin() noexcept { return m_values.rbegin(); }
	reverse_iterator rend() noexcept { return m_values.rend(); }

	const_reverse_iterator crbegin() const noexcept { return m_values.crbegin(); }
	const_reverse_iterator crend() const noexcept { return m_values.crend(); }

	// capacity

	[[nodiscard]] bool empty() const noexcept { return m_values.empty(); }
	size_type size() const noexcept { return m_values.size(); }
	difference_type ssize() const noexcept { return static_cast<difference_type>( m_values.size() ); }
	size_type max_size() const noexcept { return m_values.max_size(); }
	size_type capacity() const noexcept { return m_values.capacity(); }

	// modifiers

	void reserve( size_type s ) { m_values.reserve( s ); }

	void shrink_to_fit() { return m_values.shrink_to_fit(); }

	void clear() { m_values.clear(); }

	std::pair<iterator, bool> insert( const value_type_imp& value )
	{
		return insert( begin(), end(), value );
	}

	std::pair<iterator, bool> insert( value_type_imp&& value )
	{
		return insert( begin(), end(), std::move( value ) );
	}

	iterator insert( const_iterator hint, const value_type_imp& value )
	{
		dbExpects( cbegin() <= hint && hint <= cend() );

		if ( empty() || ( hint == end() && key_compare {}( m_values.back().first, value.first ) ) )
		{
			m_values.push_back( value );
			return begin();
		}
		else if ( key_compare{}( value.first, hint->first ) )
		{
			if ( hint == begin() || key_compare{}( ( hint - 1 )->first, value.first ) )
				return m_values.insert( hint, value );
			else
				return insert( begin(), hint, value ).first;
		}
		else
		{
			return insert( hint + 1, end(), value ).first;
		}
	}

	iterator insert( const_iterator hint, value_type_imp&& value )
	{
		dbExpects( cbegin() <= hint && hint <= cend() );

		if ( empty() || ( hint == end() && key_compare {}( m_values.back().first, value.first ) ) )
		{
			m_values.push_back( std::move( value ) );
			return begin();
		}
		else if ( key_compare{}( value.first, hint->first ) )
		{
			if ( hint == begin() || key_compare{}( ( hint - 1 )->first, value.first ) )
				return m_values.insert( hint, std::move( value ) );
			else
				return insert( begin(), hint, std::move( value ) ).first;
		}
		else
		{
			return insert( hint + 1, end(), std::move( value ) ).first;
		}
	}

	template <class InputIt>
	void insert( InputIt first, const InputIt last )
	{
		for( ; first != last; ++first )
			insert( *first );
	}

	void insert( std::initializer_list<value_type> iList )
	{
		insert( iList.begin(), iList.end() );
	}

	template <class M>
	std::pair<iterator, bool> insert_or_assign( const key_type& k, M&& obj )
	{
		auto it = lower_bound( k );
		if ( isEntry( it, k ) )
		{
			it->second = std::forward<M>( obj );
			return { it, false };
		}

		it = m_values.insert( it, value_type_imp( k, std::forward<M>( obj ) ) );
		return { it, true };
	}

	template <class M>
	std::pair<iterator, bool> insert_or_assign( key_type&& k, M&& obj) 
	{
		auto it = lower_bound( k );
		if ( isEntry( it, k ) )
		{
			it->second = std::forward<M>( obj );
			return { it, false };
		}

		it = m_values.insert( it, value_type_imp( std::move( k ), std::forward<M>( obj )));
		return { it, true };
	}

	template <class M>
	iterator insert_or_assign( const_iterator hint, const key_type& k, M&& obj )
	{
		if ( isEntry( hint, k ) )
		{
			hint->second = std::forward<M>( obj );
			return hint;
		}

		return insert( hint, k, std::forward<M>( obj ) );
	}

	template <class M>
	iterator insert_or_assign( const_iterator hint, key_type&& k, M&& obj)
	{
		if ( isEntry( hint, k ) )
		{
			hint->second = std::forward<M>( obj );
			return hint;
		}

		return insert( hint, std::move( k ), std::forward<M>( obj ) );
	}

	template <class... Args>
	std::pair<iterator, bool> emplace( Args&&... args )
	{
		auto value = value_type_imp( std::forward<Args>( args )... );
		return insert( std::move( value ) );
	}

	template <class... Args>
	iterator emplace_hint( const_iterator hint, Args&&... args )
	{
		auto value = value_type_imp( std::forward<Args>( args )... );
		return insert( hint, std::move( value ) );
	}

	template <class... Args>
	std::pair<iterator, bool> try_emplace( const key_type& k, Args&&... args ) 
	{
		auto it = lower_bound( k );
		if ( isEntry( it, k ) )
			return { it, false };

		it = m_values.insert( it, value_type_imp( k, std::forward<Args>( args )... ) );
		return { it, true };
	}

	template <class... Args>
	std::pair<iterator, bool> try_emplace( key_type&& k, Args&&... args )
	{
		auto it = lower_bound( k );
		if ( isEntry( it, k ) )
			return { it, false };

		it = m_values.insert( it, value_type_imp( std::move( k ), std::forward<Args>( args )... ) );
		return { it, true };
	}

	template <class... Args>
	iterator try_emplace( const_iterator hint, const key_type& k, Args&&... args )
	{
		if ( hint == end() || key_compare{}( k, hint->first ) )
		{
			if ( hint == begin() || key_compare{}( ( hint - 1 )->first, k ) )
				return m_values.emplace( k, T( std::forward<Args>( args )... ) );
			else
				return insert( begin(), hint, value_type_imp( k, T( std::forward<Args>( args )... ) ) ).first;
		}

		return insert( hint + 1, end(), value_type_imp( k, T( std::forward<Args>( args )... ) ) ).first;
	}
	
	template <class... Args>
	iterator try_emplace( const_iterator hint, key_type&& k, Args&&... args )
	{
		if ( hint == end() || key_compare{}( k, hint->first ) )
		{
			if ( hint == begin() || key_compare{}( ( hint - 1 )->first, k ) )
				return m_values.emplace( std::move( k ), T( std::forward<Args>( args )... ) );
			else
				return insert( begin(), hint, value_type_imp( std::move( k ), T( std::forward<Args>( args )... ) ) ).first;
		}

		return insert( hint + 1, end(), value_type_imp( std::move( k ), T( std::forward<Args>( args )... ) ) ).first;
	}

	iterator erase( const_iterator pos ) { return m_values.erase( pos ); }

	iterator erase( const_iterator first, const_iterator last ) { return m_values.erase( first, last ); }
	
	size_type erase( const key_type& key )
	{
		const auto it = find( key );
		if ( it == end() )
			return 0;

		m_values.erase( it );
		return 1;
	}

	void swap( flat_map& other ) noexcept
	{
		m_values.swap( other.m_values );
	}

	// lookup

	iterator find( const Key& key )
	{
		const auto it = lower_bound( key );
		return isEntry( it, key ) ? it : end();
	}

	const_iterator find( const Key& key ) const
	{
		const auto it = lower_bound( key );
		return isEntry( it, key ) ? it : cend();
	}

	template <class K>
	iterator find( const K& x )
	{
		const auto it = lower_bound( x );
		return isEntry( it, x ) ? it : end();
	}

	template <class K>
	const_iterator find( const K& x ) const
	{
		const auto it = lower_bound( x );
		return isEntry( it, x ) ? it : cend();
	}

	bool contains( const Key& key ) const { return find( key ) != cend(); }

	template <class K>
	bool contains( const K& x ) const { return find( x ) != cend(); }

	iterator lower_bound( const Key& key )
	{
		return std::lower_bound( begin(), end(), key, []( auto& entry, auto& key ){
			return key_compare{}( entry.first, key );
		} );
	}

	const_iterator lower_bound( const Key& key) const
	{
		return std::lower_bound( begin(), end(), key, []( auto& entry, auto& key ) {
			return key_compare{}( entry.first, key );
		} );
	}

	template <class K>
	iterator lower_bound( const K& x )
	{
		return std::lower_bound( begin(), end(), x, []( auto& entry, auto& x ) {
			return key_compare{}( entry.first, x );
		} );
	}

	template <class K>
	const_iterator lower_bound( const K& x ) const
	{
		return std::lower_bound( begin(), end(), x, []( auto& entry, auto& x ) {
			return key_compare{}( entry.first, x );
		} );
	}

	iterator upper_bound( const Key& key )
	{
		return std::upper_bound( begin(), end(), key, []( auto& entry, auto& key ){
			return key_compare{}( entry.first, key );
		} );
	}

	const_iterator upper_bound( const Key& key) const
	{
		return std::upper_bound( begin(), end(), key, []( auto& entry, auto& key ) {
			return key_compare{}( entry.first, key );
		} );
	}

	template <class K>
	iterator upper_bound( const K& x )
	{
		return std::upper_bound( begin(), end(), x, []( auto& entry, auto& x ) {
			return key_compare{}( entry.first, x );
		} );
	}

	template <class K>
	const_iterator upper_bound( const K& x ) const
	{
		return std::upper_bound( begin(), end(), x, []( auto& entry, auto& x ) {
			return key_compare{}( entry.first, x );
		} );
	}

	// observers

	constexpr key_compare key_comp() const { return key_compare{}; }

	// comparison

	friend bool operator==( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.m_values == rhs.m_values;
	}

	friend bool operator!=( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.m_values != rhs.m_values;
	}

	friend bool operator<( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.m_values < rhs.m_values;
	}

	friend bool operator>( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.m_values > rhs.m_values;
	}

	friend bool operator<=( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.m_values <= rhs.m_values;
	}

	friend bool operator>=( const flat_map& lhs, const flat_map& rhs )
	{
		return lhs.m_values >= rhs.m_values;
	}

private:
	template <typename Value>
	std::pair<iterator, bool> insert( const_iterator first, const_iterator last, Value&& value )
	{
		dbExpects( cbegin() <= first && first <= cend() );
		dbExpects( cbegin() <= last && last <= cend() );

		const auto const_it = std::lower_bound( first, last, value.first, []( auto& entry, auto& key ){
			return key_compare{}( entry.first, key );
		} );

		const iterator it = begin() + std::distance( cbegin(), const_it );

		if ( isEntry( it, value.first ) )
			return { it, false };

		return { insert( it, std::forward<Value>( value ) ), true };
	}

	template <typename I, typename K>
	constexpr bool isEntry( const I& it, const K& key ) const
	{
		dbExpects( cbegin() <= it && it <= cend() );
		return ( it != cend() ) && ( it->first == key );
	}

private:
	static constexpr const char* OUT_OF_RANGE_MESSAGE = "flat_map key out of range";

	std::vector<value_type_imp> m_values;
};

} // namespace stdx

namespace std
{
	template <class Key, class T>
	void swap( stdx::flat_map<Key, T>& lhs, stdx::flat_map<Key, T>& rhs ) noexcept
	{
		lhs.swap( rhs );
	}

	template <class Key, class T, class Pred>
	void erase_if( stdx::flat_map<Key, T>& map, Pred pred )
	{
		for ( auto it = map.begin(), last = map.end(); it != last; )
		{
			if ( pred( *it ) )
				it = map.erase( it );
			else
				++it;
		}
	}
}

#endif