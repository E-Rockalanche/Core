#pragma once

#include <stdx/assert.h>
#include <stdx/vector_s.h>

#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>

namespace Threading
{

enum class Priority : int
{
	Lowest = std::numeric_limits<int>::min(),
	Low = std::numeric_limits<int>::min() / 2,
	MediumLow = std::numeric_limits<int>::min() / 4,
	Medium = 0,
	MediumHigh = std::numeric_limits<int>::max() / 4,
	High = std::numeric_limits<int>::max() / 2,
	Highest = std::numeric_limits<int>::max()
};

class ThreadPool
{
private:
	enum class Signal
	{
		Run,
		Stop,
		Kill
	};

public:

	using Task = std::function<void()>;

	class Executor
	{
	public:
		template <typename Function>
		void Execute( Function&& f ) const
		{
			m_threadPool->QueueTask( std::forward<Function>( f ) );
		}

	private:
		Executor( ThreadPool* pool ) : m_threadPool( pool )
		{
			dbAssert( pool );
		}

		ThreadPool* m_threadPool;

		friend class ThreadPool;
	};

	explicit ThreadPool( size_t threadCount = std::thread::hardware_concurrency() )
	{
		threadCount = std::max<size_t>( threadCount, 1 );

		m_threads.reserve( threadCount );
		for ( size_t i = 0; i < threadCount; ++i )
		{
			m_threads.emplace_back( [this, i]
				{
					for(;;)
					{
						Task task;

						{
							std::unique_lock lock( m_mutex );
							m_condition.wait( lock, [this] { return m_signal != Signal::Run || !m_taskQueue.empty(); } );

							if ( ( m_signal == Signal::Stop && m_taskQueue.empty() ) || m_signal == Signal::Kill )
							{
								return;
							}

							task = Pop();
						}

						task();
					}
				} );
		}
	}

	~ThreadPool()
	{
		dbLog( "killing threads" );
		JoinWithSignal( Signal::Kill );
	}

	void QueueTask( Task task, Priority priority = Priority::Medium )
	{
		QueueTask( std::move( task ), static_cast<int>( priority ) );
	}

	void QueueTask( Task task, int priority )
	{
		{
			std::lock_guard lock( m_mutex );
			m_taskQueue.emplace( std::move( task ), priority );
		}

		m_condition.notify_one();
	}

	Executor GetExecutor()
	{
		return Executor( this );
	}

	void Join()
	{
		dbLog( "waiting to finish tasks" );
		JoinWithSignal( Signal::Stop );
	}

private:
	void JoinWithSignal( Signal s )
	{
		dbAssert( s != Signal::Run );

		{
			std::lock_guard lock( m_mutex );
			m_signal = s;
		}

		m_condition.notify_all();

		for ( auto& t : m_threads )
		{
			t.join();
		}
	}

	Task Pop()
	{
		dbAssert( !m_taskQueue.empty() );
		Task task = std::move( const_cast<Entry&>( m_taskQueue.top() ).task );
		m_taskQueue.pop();
		return task;
	}

private:

	struct Entry
	{
		Entry( Task t, int p ) : task( std::move( t ) ), priority( p ) {}

		Entry( Entry&& ) = default;
		Entry& operator=( Entry&& ) = default;

		Entry( const Entry& ) = delete;
		Entry& operator=( const Entry& ) = delete;

		Task task;
		int priority;

		constexpr bool operator<( const Entry& other ) const noexcept
		{
			return priority < other.priority;
		}
	};

	stdx::small_vector<std::thread, 16> m_threads;
	std::priority_queue<Entry> m_taskQueue;

	std::mutex m_mutex;
	std::condition_variable m_condition;

	Signal m_signal = Signal::Run;
};

extern ThreadPool StaticThreadPool;

struct ConcurrentExecutor
{
	ConcurrentExecutor() = default;
	ConcurrentExecutor( const ConcurrentExecutor& ) = default;
	ConcurrentExecutor& operator=( const ConcurrentExecutor& ) = default;

	template <typename Function>
	void Execute( Function&& f ) const
	{
		StaticThreadPool.QueueTask( std::forward<Function>( f ) );
	}

	constexpr bool operator==( const ConcurrentExecutor& ) const noexcept { return true; }
	constexpr bool operator!=( const ConcurrentExecutor& ) const noexcept { return false; }
};

}