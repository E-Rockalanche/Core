#pragma once

#include "Promise.h"
#include "SharedState.h"
#include "Task.h"

#include <memory>

namespace Threading
{

// forward declarations

template <typename T>
class Future;

template <typename T>
class SharedFuture;

template <typename T, typename Executor>
class ContinuableFuture;

template <typename T, typename Executor>
class ContinuableSharedFuture;

template <typename T>
std::pair<Future<T>, Promise<T>> MakeFuturePromisePair();

// detail

template <typename Function, typename... Args>
struct ContinuationResultOf
{
	using Type = std::decay_t<std::invoke_result_t<Function, Args...>>;
};

template <typename Function>
struct ContinuationResultOf<Function, void>
{
	using Type = std::decay_t<std::invoke_result_t<Function>>;
};

template <typename Function, typename... Args>
using ContinuationResultOf_t = typename ContinuationResultOf<Function, Args...>::Type;

template <typename StateType, typename Executor>
class BasicContinuableFuture;

template <typename StateType>
class BaseFuture
{
public:
	BaseFuture() noexcept = default;
	BaseFuture( const BaseFuture& ) = default;
	BaseFuture( BaseFuture&& ) = default;
	explicit BaseFuture( std::shared_ptr<StateType> state ) : m_state( std::move( state ) ) {}

	~BaseFuture()
	{
		if ( m_state )
			Wait();
	}

	BaseFuture& operator=( const BaseFuture& ) = default;
	BaseFuture& operator=( BaseFuture&& ) = default;

	void Detach()
	{
		m_state = nullptr;
	}

	bool Valid() const noexcept
	{
		return m_state != nullptr;
	}

	void Wait() const
	{
		m_state->Wait();
	}

protected:
	std::shared_ptr<StateType> m_state;
};

// continuable futures

template <typename Executor, typename R, typename T>
class Continuation
{
public:
	Continuation( const Executor& exec, Task<R, T> task )
		: m_executor( exec )
		, m_task( std::move( task ) )
	{}

	void operator()( Expected<T> value )
	{
		if ( value.has_value() )
		{
			if constexpr ( std::is_void_v<T> )
				m_executor.Execute( std::move( m_task ) );
			else
				m_executor.Execute( [ task = std::move( m_task ), value = std::move( value ) ]() mutable { task( std::move( value ).value() ); } );
		}
		else
		{
			m_task.SetException( std::move( value ).error() );
		}

		/*
		m_executor.Execute( [ task = std::move( m_task ), value = std::move( value ) ]() mutable
			{
				if ( value.has_value() )
				{
					if constexpr ( std::is_void_v<T> )
						task();
					else
						task( std::move( value ).value() );
				}
				else
				{
					task.SetException( std::move( value ).error() );
				}
			} );
			*/
	}

private:
	Executor m_executor;
	Task<R, T> m_task;
};

template <typename T, typename Executor>
class ContinuableFuture : public BaseFuture<SingleContinuationState<T>>
{
public:
	ContinuableFuture( std::shared_ptr<SingleContinuationState<T>> state, const Executor& exec )
		: BaseFuture<SingleContinuationState<T>>( std::move( state ) ), m_executor( exec )
	{}

	template <typename Function>
	ContinuableFuture<ContinuationResultOf_t<Function, T>, Executor> Then( Function&& f ) &&
	{
		using Result = ContinuationResultOf_t<Function, T>;
		auto state = std::make_shared<SingleContinuationState<Result>>();

		Task<Result, T> task( std::forward<Function>( f ), Promise<Result>( state ) );

		this->m_state->SetContinuation( Continuation( m_executor, std::move( task ) ) );

		this->m_state = nullptr;
		return ContinuableFuture<Result, Executor>( std::move( state ), m_executor );
	}

	operator Future<T>()
	{
		return Future<T>( std::move( this->m_state ) );
	}

private:
	Executor m_executor;
};

template <typename T, typename Executor>
class ContinuableSharedFuture : public BaseFuture<MultipleContinuationState<T>>
{
public:
	ContinuableSharedFuture( std::shared_ptr<MultipleContinuationState<T>> state, const Executor& exec )
		: BaseFuture<MultipleContinuationState<T>>( std::move( state ) ), m_executor( exec )
	{}

	template <typename Function>
	ContinuableFuture<ContinuationResultOf_t<Function, T>, Executor> Then( Function&& f ) &&
	{
		using Result = ContinuationResultOf_t<Function, T>;
		auto state = std::make_shared<SingleContinuationState<Result>>();

		Task<Result, T> task( std::forward<Function>( f ), Promise<Result>( state ) );

		this->m_state->SetContinuation( Continuation( m_executor, std::move( task ) ) );

		this->m_state = nullptr;
		return ContinuableFuture<Result, Executor>( std::move( state ), m_executor );
	}

	operator SharedFuture<T>()
	{
		return SharedFuture<T>( std::move( this->m_state ) );
	}

private:
	Executor m_executor;
};

// final future types

template <typename T>
class Future : public BaseFuture<SingleContinuationState<T>>
{
public:
	using BaseFuture<SingleContinuationState<T>>::BaseFuture;

	Future( const Future& ) = delete;
	Future( Future&& ) noexcept = default;
	Future& operator=( const Future& ) = delete;
	Future& operator=( Future&& ) noexcept = default;

	T Get()
	{
		return std::exchange( this->m_state, nullptr )->Get();
	}

	template <typename Executor>
	ContinuableFuture<T, Executor> Via( const Executor& exec ) &&
	{
		return ContinuableFuture<T, Executor>( std::move( this->m_state ), exec );
	}
};

template <>
class Future<void> : public BaseFuture<SingleContinuationState<void>>
{
public:
	using BaseFuture<SingleContinuationState<void>>::BaseFuture;

	Future( const Future& ) = delete;
	Future& operator=( const Future& ) = delete;

	void Get()
	{
		std::exchange( this->m_state, nullptr )->Get();
	}

	template <typename Executor>
	ContinuableFuture<void, Executor> Via( const Executor& exec ) &&
	{
		return ContinuableFuture<void, Executor>( std::move( this->m_state ) );
	}
};

template <typename T>
class SharedFuture : public BaseFuture<MultipleContinuationState<T>>
{
public:
	using BaseFuture<MultipleContinuationState<T>>::BaseFuture;

	T Get()
	{
		return std::exchange( this->m_state, nullptr )->Get();
	}

	template <typename Executor>
	ContinuableSharedFuture<T, Executor> Via( const Executor& exec ) &&
	{
		return ContinuableSharedFuture<T, Executor>( std::move( this->m_state ), exec );
	}
};

template <>
class SharedFuture<void> : public BaseFuture<MultipleContinuationState<void>>
{
public:
	using BaseFuture<MultipleContinuationState<void>>::BaseFuture;

	void Get()
	{
		std::exchange( this->m_state, nullptr )->Get();
	}

	template <typename Executor>
	ContinuableSharedFuture<void, Executor> Via( const Executor& exec ) &&
	{
		return ContinuableSharedFuture<void, Executor>( std::move( this->m_state ) );
	}
};

// util functions

template<typename T>
Future<T> MakeReadyFuture( T&& value )
{
	return Future<T>( std::make_shared<SingleContinuationState<T>>( std::forward<T>( value ) ) );
}

template<typename T>
SharedFuture<T> MakeReadySharedFuture( T&& value )
{
	return SharedFuture<T>( std::make_shared<MultipleContinuationState<T>>( std::forward<T>( value ) ) );
}

template <typename T>
std::pair<Future<T>, Promise<T>> MakeFuturePromisePair()
{
	auto state = std::make_shared<SingleContinuationState<T>>();
	return std::make_pair( Future<T>( state ), Promise<T>( state ) );
}

template <typename T>
std::pair<SharedFuture<T>, Promise<T>> MakeSharedFuturePromisePair()
{
	auto state = std::make_shared<MultipleContinuationState<T>>();
	return std::make_pair( SharedFuture<T>( state ), Promise<T>( state ) );
}

// Execution

template <typename Executor, typename Function>
void Execute( const Executor& exec, Function&& f )
{
	exec.Execute( std::forward<Function>( f ) );
}

template <typename Executor, typename Function>
auto TwoWayExecute( const Executor& exec, Function&& f )
{
	if ( stdx::is_detected_v )
	return exec.TwoWayExecute( std::forward<Function>( f ) ).Via( exec );
}

template <typename Executor, typename FutureType, typename Function>
auto ThenExecute( const Executor& exec, FutureType&& fut, Function&& f )
{
	return std::forward<FutureType>( fut ).Via( exec ).Then( std::forward<Function>( f ) );
}

template <typename Executor, typename Function, typename... Args>
ContinuableFuture<ContinuationResultOf_t<Function>, Executor> Async( const Executor& exec, Function&& f, Args&&... args )
{
	using Result = ContinuationResultOf_t<Function, Args...>;
	auto[ future, promise ] = MakeFuturePromisePair<Result>();

	exec.Execute( Task( std::forward<Function>( f ), std::move( promise ) ) );

	return std::move( future ).Via( exec );
}

// basic executors

struct InlineExecutor
{
	template <typename Function>
	void Execute( Function&& f ) const
	{
		std::invoke( std::forward<Function>( f ) );
	}

	constexpr bool operator==( const InlineExecutor& ) noexcept
	{
		return true;
	}

	constexpr bool operator!=( const InlineExecutor& ) noexcept
	{
		return false;
	}
};

} // namespace Threading










