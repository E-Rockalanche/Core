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
class BaseSharedState
{
	static_assert( !std::is_reference_v<T> );

public:

	using ValueType = T;
	using ErrorType = std::exception_ptr;
	using ExpectedType = stdx::expected<T, ErrorType>;
	using UnexpectedType = stdx::unexpected<ErrorType>;

	BaseSharedState() = default;

	explicit BaseSharedState( const ExpectedType& result ) : m_result( result ) {}
	explicit BaseSharedState( ExpectedType&& result ) : m_result( std::move( result ) ) {}

	explicit BaseSharedState( const UnexpectedType& result ) : m_result( result ) {}
	explicit BaseSharedState( UnexpectedType&& result ) : m_result( std::move( result ) ) {}

	template <typename... Args>
	explicit BaseSharedState( Args&&... args ) : m_result( std::in_place, std::forward<Args>( args )... ) {}

	BaseSharedState( const BaseSharedState& ) = delete;
	BaseSharedState( BaseSharedState&& ) = delete;
	BaseSharedState& operator=( const BaseSharedState& ) = delete;
	BaseSharedState& operator=( BaseSharedState&& ) = delete;

	bool IsReady() const noexcept
	{
		std::lock_guard lock( m_mutex );
		return UnsafeIsReady();
	}

	void Wait() const noexcept
	{
		std::unique_lock lock( m_mutex );
		m_condition.wait( lock, [this] { return UnsafeIsReady(); } );
	}

	template <typename Function>
	void SetContinuation( Function&& func )
	{
		std::unique_lock lock( this->m_mutex );

		if ( UnsafeIsReady() )
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
			return m_result.value().value();
	}

	decltype( auto ) Get() &&
	{
		WaitAndThrowOnError();
		if constexpr ( std::is_void_v<T> )
			return;
		else
			return std::move( m_result ).value().value();
	}

private:
	virtual bool UnsafeIsReady() const noexcept = 0;

	void WaitAndThrowOnError() const
	{
		Wait();

		if ( !m_result->has_value() )
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

protected:
	mutable std::mutex m_mutex;
	mutable std::condition_variable m_condition;
	std::optional<ExpectedType> m_result;
	stdx::small_vector<std::function<void( ExpectedType& )>, 1> m_continuations;
};

template <typename T>
class SharedState : public BaseSharedState<T>
{
public:
	using ValueType = T;
	using ErrorType = std::exception_ptr;
	using ExpectedType = stdx::expected<T, ErrorType>;
	using UnexpectedType = stdx::unexpected<ErrorType>;

	using BaseSharedState<T>::BaseSharedState;

	template <typename... Args>
	void SetExpected( Args&&... args )
	{
		{
			std::lock_guard lock( this->m_mutex );

			dbAssert( !UnsafeIsReady() );

			this->m_result.emplace( std::forward<Args>( args )... );

			this->UnsafeNotifyContinuations();
		}

		this->m_condition.notify_all();
	}

	template <typename... Args>
	void SetValue( Args&&... args )
	{
		SetExpected( std::in_place, std::forward<Args>( args )... );
	}

	void SetError( ErrorType error )
	{
		SetValue( stdx::unexpect, std::move( error ) );
	}

private:
	bool UnsafeIsReady() const noexcept final
	{
		return this->m_result.has_value();
	}
};

/*
template <typename T>
class VectorSharedState : public SharedState<std::vector<Expected<T>>>
{
public:

	using ValueType = std::vector<Expected<T>>;
	using ErrorType = std::exception_ptr;
	using ExpectedType = stdx::expected<ValueType, ErrorType>;
	using UnexpectedType = stdx::unexpected<ErrorType>;

	VectorSharedState( size_t pendingResult )
		: SharedState<std::vector<Expected<T>>>( ValueType{} )
		, m_pendingResults( pendingResult )
	{}

	void AddResult( Expected<T> expected )
	{
		std::lock_guard lock( this->m_mutex );
		dbAssert( m_pendingResults > 0 );
		this->m_result->value().push_back( std::move( expected ) );
		--m_pendingResults;
		if ( m_pendingResults == 0 )
			this->UnsafeNotifyContinuations();
	}

private:
	bool UnsafeIsReady() const noexcept final
	{
		return m_pendingResults == 0;
	}

private:
	uint32_t m_pendingResults;
};
*/

} // namespace Threading::Detail