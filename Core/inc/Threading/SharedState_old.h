#pragma once

#include <stdx/assert.h>
#include <stdx/expected.h>
#include <stdx/type_traits.h>

#include <functional>
#include <mutex>
#include <vector>

#include  "Compiler.h"

namespace Threading
{

using Error = std::exception_ptr;

template <typename T>
using Expected = stdx::expected<T, Error>;

template <typename T>
class BaseSharedState
{
public:
	static_assert( !std::is_reference_v<T> );

	BaseSharedState() = default;
	BaseSharedState( const BaseSharedState& ) = delete;
	BaseSharedState( BaseSharedState&& ) = delete;

	explicit BaseSharedState( const Expected<T>& result ) : m_result( result ) {}
	explicit BaseSharedState( Expected<T>&& result ) : m_result( std::move( result ) ) {}

	virtual ~BaseSharedState() = default;

	BaseSharedState& operator=( const BaseSharedState& ) = delete;
	BaseSharedState& operator=( BaseSharedState&& ) = delete;

	bool IsReady() const
	{
		std::lock_guard lock( m_mutex );
		return m_result.has_value();
	}

	void SetException( Error error )
	{
		dbAssert( !m_result.has_value() );
		{
			std::lock_guard lock( m_mutex );

			m_result = stdx::unexpected( std::move( error ) );

			OnValueSet();
		}
		m_condition.notify_all();
	}

	void Wait() const
	{
		std::unique_lock lock( m_mutex );
		m_condition.wait( lock, [this] { return m_result.has_value(); } );
	}

protected:
	template <typename... Args>
	void SetValueImp( Args&&... args )
	{
		dbAssert( !m_result.has_value() );

		{
			std::lock_guard lock( m_mutex );

			m_result = Expected<T>( std::forward<Args>( args )... );

			OnValueSet();
		}

		m_condition.notify_all();
	}

	virtual void OnValueSet() = 0;

protected:
	mutable std::mutex m_mutex;
	mutable std::condition_variable m_condition;
	std::optional<Expected<T>> m_result;
};

template <typename T>
class SharedStateAccess : public BaseSharedState<T>
{
public:
	using BaseSharedState<T>::BaseSharedState;

	void SetValue( const T& value )
	{
		this->SetValueImp( value );
	}

	void SetValue( T&& value )
	{
		this->SetValueImp( std::move( value ) );
	}

	T Get()
	{
		this->Wait();

		if ( !this->m_result->has_value() )
			std::rethrow_exception( std::move( *this->m_result ).error() );

		return std::move( this->m_result )->value();
	}
};

template <>
class SharedStateAccess<void> : public BaseSharedState<void>
{
public:
	using BaseSharedState<void>::BaseSharedState;

	void SetValue()
	{
		this->SetValueImp();
	}

	void Get()
	{
		this->Wait();
		if ( !this->m_result->has_value() )
			std::rethrow_exception( std::move( this->m_result )->error() );
	}
};

template <typename T>
class SingleContinuationState : public SharedStateAccess<T>
{
public:
	using SharedStateAccess<T>::SharedStateAccess;

	template <typename Function>
	void SetContinuation( Function&& f )
	{
		std::lock_guard lock( this->m_mutex );

		if ( this->m_result.has_value() )
			f( std::move( this->m_result ).value() );
		else
			m_continuation = std::forward<Function>( f );
	}

private:
	void OnValueSet() override
	{
		if ( m_continuation )
		{
			m_continuation( std::move( this->m_result ).value() );
			m_continuation = nullptr;
		}
	}

private:
	std::function<void( Expected<T> )> m_continuation;
};

template <typename T>
class MultipleContinuationState : public SharedStateAccess<T>
{
public:
	using SharedStateAccess<T>::SharedStateAccess;

	template <typename Function>
	void SetContinuation( Function&& f )
	{
		std::unique_lock lock( this->m_mutex );

		if ( this->m_result.has_value() )
		{
			lock.unlock();
			f( this->m_result.value() );
		}
		else
		{
			m_continuations.emplace_back( std::forward<Function>( f ) );
		}
	}

private:
	void OnValueSet() override
	{
		for ( auto& f : m_continuations )
			f( this->m_result.value() );

		m_continuations.clear();
	}

private:
	std::vector<std::function<void( Expected<T> )>> m_continuations;
};

} // namespace Threading