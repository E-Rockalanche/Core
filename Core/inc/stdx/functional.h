#pragma once

#include <functional>
#include <utility>

namespace stdx
{

template <typename Function, typename... FrontArgs>
class partial_bound_function
{
	static_assert( !std::is_reference_v<Function> );
	static_assert( std::conjunction_v<std::negation<std::is_reference<FrontArgs>>...> );

public:
	partial_bound_function( Function f, FrontArgs... frontArgs )
		: m_function( std::move( f ) )
		, m_frontArgs( std::move( frontArgs )... )
	{}

	template <typename... BackArgs> 
	decltype( auto ) operator()( BackArgs&&... backArgs ) &
	{
		return invoke_imp( std::index_sequence_for<FrontArgs...>{}, std::forward<BackArgs>( backArgs )... );
	}

	template <typename... BackArgs>
	decltype( auto ) operator()( BackArgs&&... backArgs ) const &
	{
		return invoke_imp( std::index_sequence_for<FrontArgs...>{}, std::forward<BackArgs>( backArgs )... );
	}

	template <typename... BackArgs>
	decltype( auto ) operator()( BackArgs&&... backArgs ) &&
	{
		return invoke_imp( std::index_sequence_for<FrontArgs...>{}, std::forward<BackArgs>( backArgs )... );
	}

private:
	template <size_t... Is, typename... BackArgs>
	decltype( auto ) invoke_imp( std::index_sequence<Is...>, BackArgs&&... backArgs ) &
	{
		return std::invoke( m_function, std::get<Is>( m_frontArgs )..., std::forward<BackArgs>( backArgs )... );
	}

	template <size_t... Is, typename... BackArgs>
	decltype( auto ) invoke_imp( std::index_sequence<Is...>, BackArgs&&... backArgs ) const &
	{
		return std::invoke( m_function, std::get<Is>( m_frontArgs )..., std::forward<BackArgs>( backArgs )... );
	}

	template <size_t... Is, typename... BackArgs>
	decltype( auto ) invoke_imp( std::index_sequence<Is...>, BackArgs&&... backArgs ) &&
	{
		return std::invoke( std::move( m_function ), std::get<Is>( std::move( m_frontArgs ) )..., std::forward<BackArgs>( backArgs )... );
	}

private:
	Function m_function;
	std::tuple<FrontArgs...> m_frontArgs;
};

template <typename Function, typename... FrontArgs>
partial_bound_function( Function, FrontArgs... ) -> partial_bound_function<std::decay_t<Function>, std::decay_t<FrontArgs>...>;

template <typename Function, typename... FrontArgs>
decltype(auto) bind( Function&& f, FrontArgs&&... frontArgs )
{
	if constexpr ( sizeof...( FrontArgs ) == 0 )
		return std::forward<Function>( f );
	else
		return partial_bound_function( std::forward<Function>( f ), std::forward<FrontArgs>( frontArgs )... );
}

}