#pragma once

#include "SharedState.h"

namespace Threading
{

template <typename T>
class BasePromise
{
public:
	using ValueType = T;

	explicit BasePromise( std::shared_ptr<SharedStateAccess<T>> state ) noexcept : m_state( std::move( state ) ) {}

	void SetException( std::exception_ptr e )
	{
		m_state->SetException( std::move( e ) );
		Detach();
	}

	bool Valid() const noexcept
	{
		return m_state != nullptr;
	}

protected:
	void Detach()
	{
		m_state = nullptr;
	}

protected:
	std::shared_ptr<SharedStateAccess<T>> m_state;
};

template <typename T>
class Promise : public BasePromise<T>
{
public:
	using BasePromise<T>::BasePromise;

	void SetValue( const T& value )
	{
		this->m_state->SetValue( value );
		this->Detach();
	}

	void SetValue( T&& value )
	{
		this->m_state->SetValue( std::move( value ) );
		this->Detach();
	}
};

template <>
class Promise<void> : public BasePromise<void>
{
public:
	using BasePromise<void>::BasePromise;

	void SetValue()
	{
		this->m_state->SetValue();
		this->Detach();
	}
};

}