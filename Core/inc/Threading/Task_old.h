#pragma once

#include "Promise.h"

#include <functional>

namespace Threading
{

template <typename R, typename... Args>
class Task
{
public:
	using FunctionType = std::function<R( Args... )>;
	using PromiseType = Promise<R>;

	Task() noexcept = default;

	Task( FunctionType f, PromiseType promise ) : m_function( std::move( f ) ), m_promise( std::move( promise ) ) {}

	void operator()( Args&&... args )
	{
		try
		{
			if constexpr ( std::is_void_v<R> )
			{
				m_function( std::forward<Args>( args )... );
				m_promise.SetValue();
			}
			else
			{
				m_promise.SetValue( m_function( std::forward<Args>( args )... ) );
			}
		}
		catch ( ... )
		{
			SetException( std::current_exception() );
		}
	}

	void SetException( Error error )
	{
		m_promise.SetException( std::move( error ) );
	}

private:
	FunctionType m_function;
	PromiseType m_promise;
};

template <typename R>
class Task<R, void>
{
public:
	using FunctionType = std::function<R()>;
	using PromiseType = Promise<void>;

	Task() noexcept = default;

	Task( FunctionType f, PromiseType promise ) : m_function( std::move( f ) ), m_promise( std::move( promise ) ) {}

	void operator()()
	{
		try
		{
			if constexpr ( std::is_void_v<R> )
			{
				m_function();
				m_promise.SetValue();
			}
			else
			{
				m_promise.SetValue( m_function() );
			}
		}
		catch ( ... )
		{
			SetException( std::current_exception() );
		}
	}

	void SetException( Error error )
	{
		m_promise.SetException( std::move( error ) );
	}

private:
	FunctionType m_function;
	PromiseType m_promise;
};

}