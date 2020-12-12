#pragma once

#include <stdx/assert.h>

#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>

namespace Threading
{

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

	ThreadPool( size_t threadCount = std::thread::hardware_concurrency() - 1 )
	{
		threadCount = std::max<size_t>( threadCount, 1 );

		m_threads.reserve( threadCount );
		for ( size_t i = 0; i < threadCount; ++i )
		{
			m_threads.push_back( std::thread( [this] {
				while ( true )
				{
					std::function<void()> task;

					{
						std::unique_lock lock( m_queueLock );
						m_condition.wait( lock, [this] { return m_signal != Signal::Run || !m_taskQueue.empty(); } );

						if ( ( m_signal == Signal::Stop && m_taskQueue.empty() ) || m_signal == Signal::Kill )
						{
							return;
						}

						task = std::move( m_taskQueue.front() );
						m_taskQueue.pop();
					}

					task();
				}
				} ) );
		}
	}

	void Join()
	{
		JoinWithSignal( Signal::Stop );
	}

	~ThreadPool()
	{
		JoinWithSignal( Signal::Kill );
	}

	template <typename F>
	void PushTask( F&& f )
	{
		{
			std::lock_guard lock( m_queueLock );
			m_taskQueue.emplace( std::forward<F>( f ) );
		}

		m_condition.notify_one();
	}



private:
	void JoinWithSignal( Signal s )
	{
		dbAssert( s != Signal::Run );

		{
			std::lock_guard lock( m_queueLock );
			m_signal = s;
		}

		m_condition.notify_all();

		for ( auto& t : m_threads )
		{
			t.join();
		}
	}

private:

	std::vector<std::thread> m_threads;
	std::queue<std::function<void()>> m_taskQueue;

	std::mutex m_queueLock;
	std::condition_variable m_condition;

	Signal m_signal = Signal::Run;
};

extern ThreadPool StaticThreadPool;

struct ConcurrentExecutor
{
	template <typename Function>
	void Execute( Function&& f ) const
	{
		StaticThreadPool.PushTask( std::forward<Function>( f ) );
	}
};

}