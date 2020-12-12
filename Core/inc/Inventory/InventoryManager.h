#pragma once

#include "Threading/Future.h"

#include <stdx/flat_map.h>
#include <stdx/reflection.h>
#include <stdx/type_traits.h>
#include <stdx/utility.h>

#include <atomic>
#include <mutex>
#include <string>
#include <typeindex>

// forward declarations

// must implement this function to load items through inventory manager
template <typename T>
T LoadInventoryItem( std::string_view filename )
{
	static_assert( "LoadInventoryItem() is not implemented for this type" );
}

template <typename T>
struct InventoryEntry;

template <typename T>
class InventoryHandle;

template <typename T>
class InventoryBucket;

class InventoryManager;

// Types

using InventoryItemHash = uint32_t;

enum class LoadState
{
	Loading,
	Ready
};

template <typename T>
struct InventoryEntry
{
	using Handle = InventoryHandle<T>;

	T item;
	mutable std::mutex mutex;
	mutable std::atomic<uint32_t> refCount = 0;
	std::string filename;
	InventoryItemHash hash = 0;
	LoadState state = LoadState::Loading;

	InventoryEntry( std::string filename_, InventoryItemHash hash_ )
		: filename( std::move( filename_ ) ), hash( hash_ )
	{}
};

template <typename T>
class InventoryHandle
{
	friend class InventoryBucket<T>;

public:
	InventoryHandle() noexcept = default;

	InventoryHandle( const InventoryHandle& other ) noexcept : m_entry{ other.m_entry }
	{
		if ( m_entry )
			++m_entry->refCount;
	}

	InventoryHandle( InventoryHandle&& other ) noexcept : m_entry{ other.m_entry }
	{
		other.m_entry = nullptr;
	}

	InventoryHandle( std::nullptr_t ) noexcept {}

	~InventoryHandle()
	{
		Reset();
	}

	InventoryHandle& operator=( const InventoryHandle& other )
	{
		Reset();
		m_entry = other.m_entry;
		if ( m_entry )
			++m_entry->refCount;

		return *this;
	}

	InventoryHandle& operator=( InventoryHandle&& other )
	{
		Reset();
		m_entry = other.m_entry;
		other.m_entry = nullptr;
		return *this;
	}

	InventoryHandle& operator=( std::nullptr_t )
	{
		Reset();
		return *this;
	}

	const T* Get() const noexcept
	{
		return std::addressof( m_entry->item );
	}

	const T* operator->() const noexcept
	{
		dbAssert( m_entry );
		return Get();
	}

	const T& operator*() const noexcept
	{
		dbAssert( m_entry );
		return m_entry->item;
	}

	bool Valid() const noexcept
	{
		return m_entry != nullptr;
	}

	void Reset();

private:
	explicit InventoryHandle( const InventoryEntry<T>* entry ) noexcept : m_entry{ entry }
	{
		dbAssert( m_entry );
		++( m_entry->refCount );
	}

private:
	const InventoryEntry<T>* m_entry = nullptr;
};

class BaseInventoryBucket
{
public:
	virtual ~BaseInventoryBucket() = default;
};

template <typename T>
class InventoryBucket : public BaseInventoryBucket
{
	static_assert( stdx::has_no_extents_v<T> );

public:
	using Handle = InventoryHandle<T>;
	using Entry = InventoryEntry<T>;

	Handle LoadSync( std::string_view filename );

	void UnloadSync( const Entry* entry );

private:
	stdx::flat_map<InventoryItemHash, std::unique_ptr<Entry>> m_items;
	std::mutex m_mutex;
};

template <typename T>
InventoryHandle<T> InventoryBucket<T>::LoadSync( std::string_view filename )
{
	const auto hash = stdx::hash_fnv1a<InventoryItemHash>( filename );

	std::unique_lock lock( m_mutex );

	auto it = m_items.find( hash );
	if ( it == m_items.end() )
	{
		// load item
		dbLog( "InventoryBucket<%s>::LoadSync( %s )", stdx::reflection::type_name_v<T>.c_str(), filename.data() );
		auto[ pos, inserted ] = m_items.insert( { hash, std::make_unique<Entry>( std::string( filename ), hash ) } );
		dbAssert( inserted );
		auto* entry = pos->second.get();
		entry->item = LoadInventoryItem<T>( filename );
		entry->state = LoadState::Ready;
		return Handle( entry );
	}
	else
	{
		dbAssertMessage( it->second->filename == filename, "detected hash collision [%s] [%s]", filename.data(), it->second->filename.c_str() );
		return Handle( it->second.get() );
	}
}

template <typename T>
void InventoryBucket<T>::UnloadSync( const Entry* entry )
{
	dbAssert( entry );
	dbLog( "InventoryBucket<%s>::UnloadSync()", stdx::reflection::type_name_v<T>.c_str() );

	std::lock_guard lock( m_mutex );

	// we may have tried to load this asset between Handle::Reset() and now
	if ( entry->refCount == 0 )
		m_items.erase( entry->hash );
	else
		dbLog( "inventory entry avoided unload [%s]", entry->filename.c_str() );
}

class InventoryManager
{
public:
	static InventoryManager* Get()
	{
		static InventoryManager s_instance;
		return &s_instance;
	}

	template <typename T>
	InventoryHandle<T> LoadSync( std::string_view filename )
	{
		return GetBucket<T>()->LoadSync( filename );
	}

	template <typename T>
	void UnloadSync( const InventoryEntry<T>* entry )
	{
		GetBucket<T>()->UnloadSync( entry );
	}

private:
	InventoryManager() = default;
	InventoryManager( const InventoryManager& ) = delete;

	template <typename T>
	InventoryBucket<T>* GetBucket();

private:
	stdx::flat_map<std::type_index, std::unique_ptr<BaseInventoryBucket>> m_buckets;
	std::mutex m_mutex;
};

template <typename T>
InventoryBucket<T>* InventoryManager::GetBucket()
{
	const auto index = std::type_index( typeid( T ) );

	std::lock_guard lock( m_mutex );

	auto it = m_buckets.find( index );
	if ( it == m_buckets.end() )
	{
		dbLog( "creating inventory bucket [%s]", stdx::reflection::type_name_v<T>.c_str() );
		it = m_buckets.insert( { index, std::make_unique<InventoryBucket<T>>() } ).first;
	}

	return static_cast<InventoryBucket<T>*>( it->second.get() );
}

template <typename T>
void InventoryHandle<T>::Reset()
{
	if ( m_entry )
	{
		// no need to be precise about ref count here. UnloadSync makes the final decision to unload
		dbAssert( m_entry->refCount > 0 );
		if ( --m_entry->refCount == 0 )
			InventoryManager::Get()->UnloadSync<T>( m_entry );

		m_entry = nullptr;
	}
}