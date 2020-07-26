#pragma once

#include <stdx/container.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

class EventListener;

enum class EventPriority : int32_t
{
	Lowest = std::numeric_limits<int32_t>::min(),
	Low = std::numeric_limits<int32_t>::min() / 2,
	Medium = 0,
	High = std::numeric_limits<int32_t>::max() / 2,
	Highest = std::numeric_limits<int32_t>::max()
};

class BaseEventSink
{
private:
	virtual void Remove( EventListener* subscriber ) noexcept = 0;

	friend class EventListener;
};

class EventListener
{
public:
	EventListener() noexcept = default;

	EventListener( const EventListener& ) = delete;
	EventListener( EventListener&& ) = delete;

	EventListener& operator=( const EventListener& ) = delete;
	EventListener& operator=( EventListener&& ) = delete;

	~EventListener()
	{
		for ( auto* eventSink : m_subscriptions )
		{
			eventSink->Remove( this );
		}
	}

	void Unsubscribe( BaseEventSink& eventSink ) noexcept
	{
		Remove( &eventSink );
		eventSink.Remove( this );
	}

private:
	std::vector<BaseEventSink*> m_subscriptions;

	void Remove( BaseEventSink* eventSink ) noexcept
	{
		const auto it = std::find( m_subscriptions.begin(), m_subscriptions.end(), eventSink );
		dbAssertMessage( it != m_subscriptions.end(), "Event sink does not exist" );
		m_subscriptions.erase( it );
	}

	friend class BaseEventSink;
	template <typename R, typename... Args> friend class EventSink;
};

namespace detail
{

template<typename FuncSig>
struct Subscription
{
	EventListener* listener;
	std::function<FuncSig> function;
	int32_t priority;

	bool operator<( const Subscription& rhs ) const noexcept
	{
		return priority > rhs.priority;
	}
};

struct ListenerPriority
{
	EventListener* listener;
	int32_t priority;
};

template <typename FuncSig>
inline Subscription<FuncSig> operator%( EventListener& listener, std::function<FuncSig>&& function ) noexcept
{
	return Subscription{ &listener, std::move( function ), static_cast<int32_t>( EventPriority::Medium ) };
}

template <typename FuncSig>
inline Subscription<FuncSig> operator%( ListenerPriority&& temp, std::function<FuncSig>&& function ) noexcept
{
	return Subscription{ &temp.listener, std::move( function ), temp.priority };
}

inline ListenerPriority operator%( EventListener& listener, EventPriority priority ) noexcept
{
	return ListenerPriority{ &listener, static_cast<int32_t>( priority ) };
}

inline ListenerPriority operator%( EventListener& listener, int32_t priority ) noexcept
{
	return ListenerPriority{ &listener, priority };
}

} // namespace detail

template <typename R, typename... Args>
class EventSink : public BaseEventSink
{
public:
	static_assert(
		std::is_same_v<void, R> || std::is_same_v<bool, R>,
		"Listener return type can only be void or bool. A bool return of false stops the event generation" );

	EventSink() noexcept = default;

	EventSink( const EventSink& ) = delete;
	EventSink( EventSink&& ) = delete;

	EventSink& operator=( const EventSink& ) = delete;
	EventSink& operator=( EventSink&& ) = delete;

	~EventSink()
	{
		for ( auto& entry : m_subscribers )
		{
			entry.listener->Remove( this );
		}
	}

	void operator+=( detail::Subscription<R(Args...)>&& subscription ) noexcept
	{
		dbAssertMessage(
			std::none_of(
				m_subscribers.begin(),
				m_subscribers.end(),
				[&]( auto& entry ) { return entry.listener == subscription.listener; } ),
			"Listener is already subscribed to this event sink" );

		const auto pos = std::lower_bound( m_subscribers.begin(), m_subscribers.end(), subscription );
		m_subscribers.insert( pos, std::move( subscription ) );
	}

	void operator()( Args... args ) noexcept
	{
		for ( auto& entry : m_subscribers )
		{
			if constexpr ( std::is_same_v<R, bool> )
			{
				if ( !entry.function( args... ) )
					break;
			}
			else
			{
				entry.function( args... );
			}
		}
	}

	uint32_t GetLowestPriority() const noexcept
	{
		return m_subscribers.empty() ? 0 : m_subscribers.back().priority;
	}

	uint32_t GetHighestPriority() const noexcept
	{
		return m_subscribers.empty() ? 0 : m_subscribers.front().priority;
	}

private:

	void Remove( EventListener* listener ) noexcept override
	{
		const auto it = std::find_if(
			m_subscribers.begin(),
			m_subscribers.end(),
			[listener]( auto& entry ) {return entry.listener == listener; } );

		dbAssertMessage( it != m_subscribers.end(), "Listener does not exist" );
		m_subscribers.erase( it );
	}

	std::vector<detail::Subscription<R( Args... )>> m_subscribers;
};