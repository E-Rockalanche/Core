#pragma once

#include <memory>
#include <vector>

template <typename Obj>
class WeakObjectPoolHandle
{
public:
	WeakObjectPoolHandle() noexcept = default;

	template <typename Obj2,
		std::enable_if_t<std::is_convertible_v<Obj2, Obj>, int> = 0>
	WeakObjectPoolHandle( const WeakObjectPoolHandle<Obj2>& other ) noexcept
		: m_object{ other.m_object }, m_generation{ other.m_generation }
	{}

	template <typename Obj2,
		std::enable_if_t<std::is_convertible_v<Obj2, Obj>, int> = 0>
	WeakObjectPoolHandle& operator=( const WeakObjectPoolHandle<Obj2>& other ) noexcept
	{
		m_object = other.m_object;
		m_generation = other.m_generation;
	}

	auto* operator->() const noexcept { return Get(); }
	auto& operator*() const noexcept { return *Get(); }

	operator bool() const noexcept
	{
		return m_object && ( m_object->generation == m_generation );
	}

	void Reset() noexcept
	{
		m_object = nullptr;
		m_generation = 0;
	}

private:
	auto* Get() const noexcept
	{
		dbAssert( m_object );
		dbAssert( m_object.generation == m_generation );
		return std::addressof( m_object->object );
	}

protected:
	Obj* m_object = nullptr;
	Generation m_generation = 0;

	template <typename Obj2>
	friend class WeakObjectPoolHandle<Obj2>;
};

template <typename ObjPool>
class UniqueObjectPoolHandle : public WeakObjectPoolHandle<typename ObjPool::ObjectAllocation>
{
public:
	UniqueObjectPoolHandle() noexcept = default;
	UniqueObjectPoolHandle( const UniqueObjectPoolHandle& ) = delete;
	UniqueObjectPoolHandle( UniqueObjectPoolHandle&& other ) noexcept
		: m_object{ std::exchange( other.m_object, nullptr ) }, m_generation{ other.m_generation }
	{}

	UniqueObjectPoolHandle& operator=( const UniqueObjectPoolHandle& ) = delete;
	UniqueObjectPoolHandle& operator=( UniqueObjectPoolHandle&& other ) noexcept
	{
		m_object = std::exchange( other.m_object, nullptr );
		m_generation = other.m_generation;
		return *this;
	}

	~UniqueObjectPoolHandle()
	{
		Reset();
	}

	void Reset() noexcept
	{
		if ( m_object )
		{
			ObjPool::Get()->Destroy( m_object );
			m_object = nullptr;
			m_generation = 0;
		}
	}

private:
	UniqueObjectPoolHandle( ObjAllocation* object, Generation generation ) noexcept
		: m_object{ object }, m_generation{ generation }
	{}

	friend class ObjPool;
};

template <typename T>
class ObjectPool
{
	static_assert( std::is_unsigned_v<Generation> && std::is_integral_v<Generation> );

public:
	using Generation = uint16_t;
	
	struct ObjectAllocation
	{
		T object;
		Generation generation;
	};

	using WeakHandle = WeakObjectPoolHandle<ObjectAllocation>;
	using ConstWeakHandle = WeakObjectPoolHandle<const ObjectAllocation>;
	using UniqueHandle = UniqueObjectPoolHandle<ObjectPool>;

	static ObjectPool* Get() noexcept
	{
		static ObjectPool s_pool;
		return &s_pool;
	}

	template <typename... Args>
	UniqueHandle Create( Args&&... args );

private:

	ObjectPool() = default;
	~ObjectPool();

	ObjectAllocation* Allocate();
	void Destroy( ObjectAllocation* object );

private:
	std::vector<ObjectAllocation*> m_freeList;
	size_t m_totalAllocated = 0;
};

template <typename T>
template <typename... Args>
typename ObjectPool<T>::UniqueHandle ObjectPool<T>::Create( Args&&... args )
{
	ObjectAllocation* object = Allocate();
	new( std::addressof( object->object ) ) T( std::forward<Args>( args )... );
	return UniqueHandle( object, object->generation );
}

template <typename T>
typename ObjectPool<T>::ObjectAllocation* ObjectPool<T>::Allocate()
{
	if ( m_freeList.empty() )
	{
		void* mem = ::operator new( sizeof( ObjectAllocation ) );
		auto* object = static_cast<ObjectAllocation*>( mem );
		object->generation = 0;
		++m_totalAllocated;
		return object;
	}
	else
	{
		auto* object = m_freeList.back();
		m_freeList.pop_back();
		return object;
	}
}

template <typename T>
void ObjectPool<T>::Destroy( ObjectAllocation* object )
{
	object->object~T();
	object->generation++;
	m_freeList.push_back( object );
}

template <typename T>
ObjectPool<T>::~ObjectPool()
{
	dbAssertMessage( m_totalAllocated == m_freeList.size(), "handle lifetimes exceeded pool lifetime" );

	for ( auto* object : m_freeList )
		::operator delete( object );
}