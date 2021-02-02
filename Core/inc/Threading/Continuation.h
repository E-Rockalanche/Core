#pragma once

#include "Execution.h"
#include "Promise.h"

#include <stdx/type_traits.h>

namespace Threading
{

using Error = std::exception_ptr;

template <typename T>
using Expected = stdx::expected<T, Error>;

using Unexpected = stdx::unexpected<Error>;

template <typename ValueFunc>
class OnValue
{
public:
	OnValue( ValueFunc&& f ) : m_onValue( std::forward<ValueFunc>( f ) ) {}

	template <typename Arg>
	decltype( auto ) Value( Arg&& arg )
	{
		return m_onValue( std::forward<Arg>( arg ) );
	}

private:
	ValueFunc m_onValue;
};

template <typename ValueFunc>
OnValue( ValueFunc&& )->OnValue<std::decay_t<ValueFunc>>;

template <typename DoneFunc>
class OnDone
{
public:
	OnDone( DoneFunc&& f ) : m_onDone( std::forward<DoneFunc>( f ) ) {}

	decltype( auto ) Done()
	{
		return m_onDone();
	}

private:
	DoneFunc m_onDone;
};

template <typename DoneFunc>
OnDone( DoneFunc&& )->OnDone<std::decay_t<DoneFunc>>;

template <typename ErrorFunc>
class OnError
{
public:
	OnError( ErrorFunc&& f ) : m_onError( std::forward<ErrorFunc>( f ) ) {}

	void Error( Error error )
	{
		m_onError( std::move( error ) );
	}

private:
	ErrorFunc m_onError;
};

template <typename ErrorFunc>
OnError( ErrorFunc&& )->OnError<std::decay_t<ErrorFunc>>;

template <typename ExpectedFunc>
class OnExpected
{
public:
	OnExpected( ExpectedFunc&& f ) : m_onExpected( std::forward<ExpectedFunc>( f ) ) {}

	template <typename E>
	decltype( auto ) Expected( E&& e )
	{
		return m_onExpected( std::forward<E>( e ) );
	}

private:
	ExpectedFunc m_onExpected;
};

template <typename ExpectedFunc>
OnExpected( ExpectedFunc&& )->OnExpected<std::decay_t<ExpectedFunc>>;

template <typename ValueFunc, typename ErrorFunc>
class OnValueOrError : public OnValue<ValueFunc>, public OnError<ErrorFunc>
{
	OnValueOrError( ValueFunc&& onValue, ErrorFunc&& onError )
		: OnValue<ValueFunc>( std::forward<ValueFunc>( onValue ) )
		, OnError<ErrorFunc>( std::forward<ErrorFunc>( onError ) )
	{}
};

template <typename ValueFunc, typename ErrorFunc>
OnValueOrError( ValueFunc&&, ErrorFunc&& )->OnValueOrError<std::decay_t<ValueFunc>, std::decay_t<ErrorFunc>>;

template <typename DoneFunc, typename ErrorFunc>
class OnDoneOrError : public OnDone<DoneFunc>, public OnError<ErrorFunc>
{
	OnDoneOrError( DoneFunc&& onValue, ErrorFunc&& onError )
		: OnDone<DoneFunc>( std::forward<DoneFunc>( onValue ) )
		, OnError<ErrorFunc>( std::forward<ErrorFunc>( onError ) )
	{}
};

template <typename DoneFunc, typename ErrorFunc>
OnDoneOrError( DoneFunc&&, ErrorFunc&& )->OnDoneOrError<std::decay_t<DoneFunc>, std::decay_t<ErrorFunc>>;

namespace Detail
{

	template <typename T, typename Arg>
	using OnValueType = decltype( std::declval<T>().Value( std::declval<Arg>() ) );

	template <typename T>
	using OnErrorType = decltype( std::declval<T>().Error( std::declval<Error&&>() ) );

	template <typename T>
	using OnDoneType = decltype( std::declval<T>().Done() );

	template <typename T, typename E>
	using OnExpectedType = decltype( std::declval<T>().Expected( std::declval<E>() ) );

	template <typename T, typename Arg>
	inline constexpr bool HasValueFunc_v = stdx::is_detected_v<Detail::OnValueType, T, Arg>;

	template <typename T>
	inline constexpr bool HasErrorFunc_v = stdx::is_detected_v<Detail::OnErrorType, T>;

	template <typename T>
	inline constexpr bool HasDoneFunc_v = stdx::is_detected_v<Detail::OnDoneType, T>;

	template <typename T, typename E>
	inline constexpr bool HasExpectedFunc_v = stdx::is_detected_v<Detail::OnExpectedType, T, E>;

	template <typename Function, typename T>
	class BoundReceiver
	{
		static_assert( !std::is_reference_v<Function> );
		static_assert( !std::is_reference_v<T> );
		static_assert( HasValueFunc_v<Function, T&&> || HasInvokeOperator_v<Function, T&&> );

	public:
		BoundReceiver( Function f, T value )
			: m_function( std::move( f ) )
			, m_value( std::move( value ) )
		{}

		decltype( auto ) operator()()
		{
			if constexpr ( HasExpectedFunc_v<Function&&, T&&> )
				return std::move( m_function ).Expected( std::move( m_value ) );
			if constexpr ( HasValueFunc_v<Function&&, T&&> )
				return std::move( m_function ).Value( std::move( m_value ) );
			else if constexpr ( HasInvokeOperator_v<Function&&, T&&> )
				return std::move( m_function )( std::move( m_value ) );
			else
				static_assert( false, "function cannot be invoked with T" );
		}

	private:
		T m_value;
		Function m_function;
	};

	template <typename Function>
	class BoundReceiver<Function, void>
	{
		static_assert( !std::is_reference_v<Function> );

	public:
		BoundReceiver( Function f )
			: m_function( std::move( f ) )
		{}

		decltype( auto ) operator()()
		{
			if constexpr ( HasDoneFunc_v<Function&&> )
				return std::move( m_function ).Done();
			else if constexpr ( HasInvokeOperator_v<Function&&> )
				return std::move( m_function )();
			else
				static_assert( false, "function cannot be invoked with no parameters" );
		}

	private:
		Function m_function;
	};

	template <typename Function, typename T>
	BoundReceiver( Function, T )->BoundReceiver<std::decay_t<Function>, std::decay_t<T>>;

	template <typename Function>
	BoundReceiver( Function )->BoundReceiver<std::decay_t<Function>, void>;

	template <typename Function, typename ExpectedType>
	auto BindReceiver( Function&& f, ExpectedType&& expected )
	{
		if constexpr ( HasExpectedFunc_v<Function&&, ExpectedType&&> )
		{
			return BoundReceiver( std::forward<Function>( f ), std::forward<ExpectedType>( expected ) );
		}
		else if constexpr ( std::is_void_v<typename std::decay_t<ExpectedType>::value_type> )
		{
			dbAssert( expected.has_value() );
			return BoundReceiver( std::forward<Function>( f ) );
		}
		else
		{
			dbAssert( expected.has_value() );
			return BoundReceiver( std::forward<Function>( f ), std::forward<ExpectedType>( expected ).value() );
		}
	}

	template <typename Function, typename T>
	using BindReceiver_t = decltype( BindReceiver( std::declval<Function>(), std::declval<Expected<T>>() ) );

	template <typename Function, typename T>
	using ContinuationResultType_t = std::decay_t<std::invoke_result_t<BindReceiver_t<Function, T>>>;

	template <typename R, typename Function, typename... Args>
	void InvokeContinuation( Promise<R> promise, Function&& f, Args&&... args ) noexcept
	{
		static_assert( std::is_same_v<R, std::decay_t<std::invoke_result_t<Function&&, Args&&...>>> );

		try
		{
			if constexpr ( std::is_void_v<R> )
			{
				std::invoke( std::forward<Function>( f ), std::forward<Args>( args )... );
				promise.SetValue();
			}
			else
			{
				promise.SetValue( std::invoke( std::forward<Function>( f ), std::forward<Args>( args )... ) );
			}
		}
		catch ( ... )
		{
			promise.SetError( std::current_exception() );
		}
	}

	template <typename R, typename Function>
	class Task
	{
		static_assert( !std::is_reference_v<R> );
		static_assert( !std::is_reference_v<Function> );

	public:
		Task( Promise<R> promise, Function f )
			: m_promise( std::move( promise ) )
			, m_function( std::move( f ) )
		{}

		void operator()() noexcept
		{
			InvokeContinuation( m_promise, m_function );
		}

	private:
		Promise<R> m_promise;
		Function m_function;
	};

	template <typename R, typename Function>
	Task( Promise<R>, Function )->Task<R, std::decay_t<Function>>;

	template <typename R, typename Function>
	class ErrorTask
	{
		static_assert( !std::is_reference_v<R> );
		static_assert( !std::is_reference_v<Function> );

	public:
		ErrorTask( Promise<R> promise, Function f, Error error )
			: m_promise( std::move( promise ) )
			, m_error( std::move( error ) )
			, m_function( std::move( f ) )
		{}

		void operator()() noexcept
		{
			try
			{
				std::invoke( m_function, m_error );
			}
			catch ( ... )
			{
				dbLogError( "exception thrown while handling continuation error" );
			}
			m_promise.SetError( std::move( m_error ) );
		}

	private:
		Promise<R> m_promise;
		Error m_error;
		Function m_function;
	};

	template <typename R, typename Function>
	ErrorTask( Promise<R>, Function, Error )->ErrorTask<R, std::decay_t<Function>>;

	template <typename Qualifier, typename R, typename Executor, typename Function>
	class BoundContinuation
	{
		static_assert( !std::is_reference_v<R> );
		static_assert( !std::is_reference_v<Executor> );
		static_assert( !std::is_reference_v<Function> );

	public:
		BoundContinuation( Qualifier, Promise<R> promise, const Executor& exec, Function&& f )
			: m_promise( std::move( promise ) )
			, m_executor( exec )
			, m_function( std::move( f ) )
		{}

		template <typename T>
		void operator()( Expected<T>& expected )
		{
			if constexpr ( HasExpectedFunc_v<Function, Expected<T>&&> )
			{
				Execute( this->m_executor, Task( std::move( this->m_promise ), BindReceiver( std::move( this->m_function ), Qualifier{}( expected ) ) ) );
			}
			else if ( expected.has_value() )
			{
				Execute( this->m_executor, Task( std::move( this->m_promise ), BindReceiver( std::move( this->m_function ), Qualifier{}( expected ) ) ) );
			}
			else if constexpr ( HasErrorFunc_v<Function> )
			{
				Execute( this->m_executor, ErrorTask( std::move( this->m_promise ), std::move( this->m_function ), Qualifier{}( expected ).error() ) );
			}
			else
			{
				this->m_promise.SetError( Qualifier{}( expected ).error() );
			}
		}

	protected:
		Promise<R> m_promise;
		Executor m_executor;
		Function m_function;
	};

	template <typename Qualifier, typename R, typename Executor, typename Function>
	BoundContinuation( Qualifier, Promise<R>, const Executor&, Function&& )->BoundContinuation<std::decay_t<Qualifier>, R, Executor, std::decay_t<Function>>;

	struct MoveQualifier
	{
		template <typename Arg>
		auto&& operator()( Arg&& arg )
		{
			return std::move( arg );
		}
	};

	struct ConstQualifier
	{
		template <typename Arg>
		const auto& operator()( Arg&& arg )
		{
			return std::as_const( arg );
		}
	};

} // namespace Detail

}