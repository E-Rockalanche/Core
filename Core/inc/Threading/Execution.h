#pragma once

#include <stdx/functional.h>
#include <stdx/type_traits.h>

namespace Threading
{

struct InlineExecutor
{
	template <typename Function, typename... Args>
	void Execute( Function&& f, Args&&... args ) const
	{
		std::invoke( std::forward<Function>( f ), std::forward<Args>( args )... );
	}

	constexpr bool operator==( const InlineExecutor& ) const noexcept { return true; }
	constexpr bool operator!=( const InlineExecutor& ) const noexcept { return false; }
};

namespace Detail
{
	template <typename Exec, typename Function, typename... Args>
	using ExecuteType = decltype( std::declval<const Exec>().Execute( std::declval<Function>(), std::declval<Args>()... ) );

	template <typename Exec, typename Function, typename... Args>
	using TwoWayExecuteType = decltype( std::declval<const Exec>().TwoWayExecute( std::declval<Function>(), std::declval<Args>()... ) );
}

template<typename Exec, typename Function, typename... Args>
inline constexpr bool HasExecute_v = stdx::is_detected_v<Detail::ExecuteType, Exec, Function, Args...>;

template<typename Exec, typename Function, typename... Args>
inline constexpr bool HasTwoWayExecute_v = stdx::is_detected_v<Detail::TwoWayExecuteType, Exec, Function, Args...>;

template <typename T, typename... Args>
inline constexpr bool HasInvokeOperator_v = stdx::is_detected_v<std::invoke_result_t, T, Args...>;

template <typename Executor, typename Function, typename... Args>
inline void Execute( const Executor& exec, Function&& f, Args&&... args )
{
	using BindType = decltype( stdx::bind( std::forward<Function>( f ), std::forward<Args>( args )... ) );

	if constexpr ( HasExecute_v<Executor, Function&&, Args&&...> )
		exec.Execute( std::forward<Function>( f ), std::forward<Args>( args )... );
	else if constexpr ( HasExecute_v<Executor, BindType> )
		exec.Execute( stdx::bind( std::forward<Function>( f ), std::forward<Args>( args )... ) );
	else if constexpr ( HasInvokeOperator_v<Executor, Function&&, Args&&...> )
		exec( std::forward<Function>( f ), std::forward<Args>( args )... );
	else if constexpr ( HasInvokeOperator_v<Executor, BindType> )
		exec( stdx::bind( std::forward<Function>( f ), std::forward<Args>( args )... ) );
	else if constexpr ( HasTwoWayExecute_v<Executor, Function&&, Args&&...> )
		exec.TwoWayExecute( std::forward<Function>( f ), std::forward<Args>( args )... ).Discard();
	else if constexpr ( HasTwoWayExecute_v<Executor, BindType> )
		exec.TwoWayExecute( stdx::bind( std::forward<Function>( f ), std::forward<Args>( args )... ) ).Discard();
	else
		static_assert( false, "cannot invoke executor" );
}

}