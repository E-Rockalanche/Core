#pragma once

#include <stdx/assert.h>
#include <stdx/expected.h>
#include <stdx/type_traits.h>
#include <stdx/vector_s.h>

#include <functional>
#include <mutex>

namespace Threading::Detail
{

template <typename T>
class SharedState
{
	static_assert( !std::is_reference_v<T> );

public:

	using ValueType = T;
	using ErrorType = std::exception_ptr;
	using ExpectedType = stdx::expected<T, ErrorType>;
	using UnexpectedType = stdx::unexpected<ErrorType>;

	SharedState() = default;

	explicit SharedState( const ExpectedType& result ) : m_result( result ) {}
	explicit SharedState( ExpectedType&& result ) : m_result( std::move( result ) ) {}

	explicit SharedState( const UnexpectedType& result ) : m_result( result ) {}
	explicit SharedState( UnexpectedType&& result ) : m_result( std::move( result ) ) {}

	template <typename... Args>
	explicit SharedState( Args&&... args ) : m_result( std::in_place, std::forward<Args>( args )... ) {}

	SharedState( const SharedState& ) = delete;
	SharedState( SharedState&& ) = delete;
	SharedState& operator=( const SharedState& ) = delete;
	SharedState& operator=( SharedState&& ) = delete;

	bool IsReady() const noexcept
	{
		std::lock_guard lock( m_mutex );
		return m_result.has_value();
	}

	void SetError( ErrorType error )
	{
		dbAssert( !m_result.has_value() );
		{
			std::lock_guard lock( m_mutex );

			m_result = stdx::unexpected( std::move( error ) );

			UnsafeNotifyContinuations();
		}
		m_condition.notify_all();
	}

	void Wait() const noexcept
	{
		std::unique_lock lock( m_mutex );
		m_condition.wait( lock, [this] { return m_result.has_value(); } );
	}

	template <typename... Args>
	void SetValue( Args&&... args )
	{
		dbAssert( !m_result.has_value() );

		{
			std::lock_guard lock( m_mutex );

			m_result = ExpectedType( std::forward<Args>( args )... );

			UnsafeNotifyContinuations();
		}

		m_condition.notify_all();
	}

	template <typename E = ExpectedType>
	void SetExpected( E&& result )
	{
		dbAssert( !m_result.has_value() );

		{
			std::lock_guard lock( m_mutex );

			m_result = std::forward<E>( result );

			UnsafeNotifyContinuations();
		}

		m_condition.notify_all();
	}

	template <typename Function>
	void SetContinuation( Function&& func )
	{
		std::unique_lock lock( this->m_mutex );

		if ( m_result.has_value() )
		{
			lock.unlock();
			std::invoke( std::forward<Function>( func ), *m_result );
		}
		else
		{
			m_continuations.emplace_back( std::forward<Function>( func ) );
		}
	}

	decltype( auto ) Get() const &
	{
		WaitAndThrowOnError();
		if constexpr ( std::is_void_v<T> )
			return;
		else
			return this->m_result->value();
	}

	decltype( auto ) Get() &&
	{
		WaitAndThrowOnError();
		if constexpr ( std::is_void_v<T> )
			return;
		else
			return std::move( this->m_result )->value();
	}

private:
	void WaitAndThrowOnError() const
	{
		this->Wait();

		if ( !this->m_result->has_value() )
			std::rethrow_exception( std::move( *this->m_result ).error() );
	}

	void UnsafeNotifyContinuations()
	{
		for ( auto& func : m_continuations )
		{
			func( *m_result );
		}
		m_continuations.clear();
	}

private:
	mutable std::mutex m_mutex;
	mutable std::condition_variable m_condition;
	std::optional<ExpectedType> m_result;
	stdx::small_vector<std::function<void( ExpectedType& )>, 1> m_continuations;
};

} // namespace Threading::Detail