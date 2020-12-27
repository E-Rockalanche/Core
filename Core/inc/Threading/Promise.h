#pragma once

#include "SharedState.h"

namespace Threading
{

template <typename T>
class Promise
{
public:
	using ValueType = T;
	using StateType = Detail::SharedState<T>;
	using ErrorType = typename StateType::ErrorType;
	using ExpectedType = typename StateType::ExpectedType;

	explicit Promise( std::shared_ptr<StateType> state ) noexcept : m_state( std::move( state ) ) {}

	bool Valid() const noexcept
	{
		return m_state != nullptr;
	}

	void SetError( ErrorType e )
	{
		dbAssert( Valid() );
		m_state->SetError( std::move( e ) );
		m_state = nullptr;
	}

	template <typename... Args>
	void SetValue( Args&&... args )
	{
		dbAssert( Valid() );
		m_state->SetValue( std::forward<Args>( args )... );
		m_state = nullptr;
	}

	template <typename E = ExpectedType>
	void SetExpected( E&& expected )
	{
		dbAssert( Valid() );
		m_state->SetExpected( std::forward<E>( expected ) );
		m_state = nullptr;
	}

protected:
	std::shared_ptr<StateType> m_state;
};

} // namespace Threading