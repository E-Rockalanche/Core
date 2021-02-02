#pragma once

#include <stdx/compiler.h>

#include "Continuation.h"
#include "Execution.h"
#include "SharedState.h"
#include "ThreadPool.h"

#include <stdx/container.h>
#include <stdx/functional.h>

namespace Threading
{

template <typename T>
class BaseFuture;

template <typename T>
class Future;

template <typename T>
class SharedFuture;

template <typename T, typename Exec>
class ContinuableFuture;

template <typename T, typename Exec>
class ContinuableSharedFuture;

template <typename T, typename... Args>
Future<T> MakeReadyFuture( Args&&... args );

template <typename T, typename... Args>
SharedFuture<T> MakeReadySharedFuture( Args&&... args );

template <typename T>
std::pair<Future<T>, Promise<T>> MakeFuturePromisePair();

template <typename T>
std::pair<SharedFuture<T>, Promise<T>> MakeSharedFuturePromisePair();

// type traits

template <typename T>
struct IsFuture : std::false_type {};

template <typename T>
struct IsFuture<Future<T>> : std::true_type {};

template <typename T>
struct IsFuture<SharedFuture<T>> : std::true_type {};

template <typename T, typename Exec>
struct IsFuture<ContinuableFuture<T, Exec>> : std::true_type {};

template <typename T, typename Exec>
struct IsFuture<ContinuableSharedFuture<T, Exec>> : std::true_type {};

template <typename T>
inline constexpr bool IsFuture_v = IsFuture<T>::value;

template <typename T>
struct IsSharedFuture : std::false_type {};

template <typename T>
struct IsSharedFuture<SharedFuture<T>> : std::true_type {};

template <typename T, typename Exec>
struct IsSharedFuture<ContinuableSharedFuture<T, Exec>> : std::true_type {};

template <typename T>
inline constexpr bool IsSharedFuture_v = IsSharedFuture<T>::value;

template <typename T>
struct IsContinuableFuture : std::false_type {};

template <typename T, typename Exec>
struct IsContinuableFuture<ContinuableFuture<T, Exec>> : std::true_type {};

template <typename T, typename Exec>
struct IsContinuableFuture<ContinuableSharedFuture<T, Exec>> : std::true_type {};

template <typename T>
inline constexpr bool IsContinuableFuture_v = IsContinuableFuture<T>::value;

template <typename T>
struct IsContinuableSharedFuture : std::false_type {};

template <typename T, typename Exec>
struct IsContinuableSharedFuture<ContinuableSharedFuture<T, Exec>> : std::true_type {};

template <typename T>
inline constexpr bool IsContinuableSharedFuture_v = IsContinuableSharedFuture<T>::value;

template <typename T>
struct IsNestedFuture : std::false_type {};

template <typename T>
struct IsNestedFuture<Future<T>> : IsFuture<T> {};

template <typename T>
struct IsNestedFuture<SharedFuture<T>> : IsFuture<T> {};

template <typename T, typename Exec>
struct IsNestedFuture<ContinuableFuture<T, Exec>> : IsFuture<T> {};

template <typename T, typename Exec>
struct IsNestedFuture<ContinuableSharedFuture<T, Exec>> : IsFuture<T> {};

template <typename T>
inline constexpr bool IsNestedFuture_v = IsNestedFuture<T>::value;

template <typename T>
struct RemoveExecutor {};

template <typename T, typename Exec>
struct RemoveExecutor<ContinuableFuture<T, Exec>>
{
	using type = Future<T>;
};

template <typename T, typename Exec>
struct RemoveExecutor<ContinuableSharedFuture<T, Exec>>
{
	using type = SharedFuture<T>;
};

template <typename T>
using RemoveExecutor_t = typename RemoveExecutor<T>::type;

template <typename T, typename Executor>
struct AddExecutor {};

template <typename T, typename Executor>
struct AddExecutor<Future<T>, Executor>
{
	using type = ContinuableFuture<T, Executor>;
};

template <typename T, typename Executor>
struct AddExecutor<SharedFuture<T>, Executor>
{
	using type = ContinuableSharedFuture<T, Executor>;
};

template <typename T, typename OldExecutor, typename Executor>
struct AddExecutor<ContinuableFuture<T, OldExecutor>, Executor>
{
	using type = ContinuableFuture<T, Executor>;
};

template <typename T, typename OldExecutor, typename Executor>
struct AddExecutor<ContinuableSharedFuture<T, OldExecutor>, Executor>
{
	using type = ContinuableSharedFuture<T, Executor>;
};

template <typename T, typename Executor>
using AddExecutor_t = typename AddExecutor<T, Executor>::type;

template <typename T>
struct FutureValueType : std::enable_if<IsFuture_v<T>, typename T::ValueType> {};

template <typename T>
using FutureValueType_t = typename FutureValueType<T>::type;

template <typename T>
struct IsFutureReference : std::conjunction<IsFuture<std::decay_t<T>>, std::is_reference<T>> {};

template <typename T>
inline constexpr bool IsFutureReference_v = IsFutureReference<T>::value;

template <typename T>
struct IsFutureRValue : std::conjunction<IsFuture<std::decay_t<T>>, std::is_rvalue_reference<T>, std::negation<std::is_const<T>>> {};

template <typename T>
inline constexpr bool IsFutureRValue_v = IsFutureRValue::value;

template <typename T>
struct IsVoidFuture : std::false_type {};

template<>
struct IsVoidFuture<Future<void>> : std::true_type {};

template<>
struct IsVoidFuture<SharedFuture<void>> : std::true_type {};

template <typename Exec>
struct IsVoidFuture<ContinuableFuture<void, Exec>> : std::true_type {};

template <typename Exec>
struct IsVoidFuture<ContinuableSharedFuture<void, Exec>> : std::true_type {};

template <typename T>
inline constexpr bool IsVoidFuture_v = IsVoidFuture<T>::value;

// future implementation

namespace Detail
{


template <typename T>
class [[nodiscard]] BaseFuture
{
public:
	using ValueType = T;
	using ExpectedType = Expected<T>;
	using StateType = Detail::SharedState<T>;

	BaseFuture() noexcept = default;
	BaseFuture( const BaseFuture& ) noexcept = default;
	BaseFuture( BaseFuture&& ) noexcept = default;

	explicit BaseFuture( std::shared_ptr<StateType> state ) noexcept : m_state( std::move( state ) ) {}

	~BaseFuture()
	{
		if ( m_state )
			Wait();
	}

	BaseFuture& operator=( const BaseFuture& ) = default;
	BaseFuture& operator=( BaseFuture&& ) = default;

	void Discard()
	{
		m_state = nullptr;
	}

	bool Valid() const noexcept
	{
		return m_state != nullptr;
	}

	void Wait() const noexcept
	{
		dbAssert( m_state );
		m_state->Wait();
	}

	bool IsReady() const noexcept
	{
		dbAssert( m_state );
		return m_state->IsReady();
	}

protected:
	std::shared_ptr<StateType> m_state;
};

}

template <typename T>
class Future : public Detail::BaseFuture<T>
{
public:
	using ValueType = T;
	using ExpectedType = Expected<T>;

	using Detail::BaseFuture<T>::BaseFuture;

	Future( const Future& ) = delete;
	Future& operator=( const Future& ) = delete;

	Future( Future&& ) = default;
	Future& operator=( Future&& ) = default;

	decltype( auto ) Get() &&
	{
		this->Wait();
		return std::move( *std::exchange( this->m_state, nullptr ) ).Get();
	}

	SharedFuture<T> Share() && noexcept;

	template <typename Exec>
	ContinuableFuture<T, Exec> Via( const Exec& executor ) &&;

	template <typename U = T, std::enable_if_t<IsFuture_v<U>, int> = 0>
	auto Unwrap() && -> RemoveExecutor_t<U>;
};

template <typename T>
template <typename U, std::enable_if_t<IsFuture_v<U>, int>>
auto Future<T>::Unwrap() && -> RemoveExecutor_t<U>
{
	auto[ future, promise ] = MakeFuturePromisePair<typename U::ValueType>();
	std::move( *this ).Via( InlineExecutor() ).Then( OnExpected{ [promise = std::move( promise )]( ExpectedType&& expectedFuture ) mutable
	{
		if ( expectedFuture.has_value() )
			std::move( expectedFuture ).value().Then( OnExpected{ [promise = std::move( promise )]( typename U::ExpectedType&& expectedValue ) mutable
			{
				promise.SetExpected( std::move( expectedValue ) );
			} } );
		else
			promise.SetError( std::move( expectedFuture ).error() );
	} }  );
	return std::move( future );
}

template <typename T>
class SharedFuture : public Detail::BaseFuture<T>
{
public:
	using ValueType = T;
	using ExpectedType = Expected<T>;

	using Detail::BaseFuture<T>::BaseFuture;

	decltype( auto ) Get() &&
	{
		this->Wait();
		return std::exchange( this->m_state, nullptr )->Get();
	}

	template <typename Exec>
	ContinuableSharedFuture<T, Exec> Via( const Exec& executor ) &&;

	template <typename U = T, std::enable_if_t<IsFuture_v<U>, int> = 0>
	auto Unwrap() && -> RemoveExecutor_t<U>;
};

template <typename T>
template <typename U, std::enable_if_t<IsFuture_v<U>, int>>
auto SharedFuture<T>::Unwrap() && -> RemoveExecutor_t<U>
{
	auto[ future, promise ] = MakeFuturePromisePair<typename U::ValueType>();
	std::move( *this ).Via( InlineExecutor() ).Then( OnExpected{ [promise = std::move( promise )]( const ExpectedType& expectedFuture ) mutable
	{
		if ( expectedFuture.has_value() )
			expectedFuture.value().Then( OnExpected{ [promise = std::move( promise )]( typename U::ExpectedType&& expectedValue ) mutable
			{
				promise.SetExpected( std::move( expectedValue ) );
			} } );
		else
			promise.SetError( expectedFuture.error() );
	} } );
	return std::move( future );
}

template <typename T>
SharedFuture<T> Future<T>::Share() && noexcept
{
	return SharedFuture<T>( std::move( this->m_state ) );
}

template<typename T, typename... Args>
inline Future<T> MakeReadyFuture( Args&&... args )
{
	return Future<T>( std::make_shared<Detail::SharedState<T>>( std::forward<Args>( args )... ) );
}

template<typename T, typename... Args>
inline SharedFuture<T> MakeReadySharedFuture( Args&&... args )
{
	return SharedFuture<T>( std::make_shared<Detail::SharedState<T>>( std::forward<Args>( args )... ) );
}

template <typename T>
inline std::pair<Future<T>, Promise<T>> MakeFuturePromisePair()
{
	auto state = std::make_shared<Detail::SharedState<T>>();
	return std::make_pair( Future<T>( state ), Promise<T>( state ) );
}

template <typename T>
inline std::pair<SharedFuture<T>, Promise<T>> MakeSharedFuturePromisePair()
{
	auto state = std::make_shared<Detail::SharedState<T>>();
	return std::make_pair( SharedFuture<T>( state ), Promise<T>( state ) );
}

namespace Detail
{

template <typename FutureBase, typename Exec, typename Qualifier>
class BaseContinuableFuture : public FutureBase
{
public:
	using ValueType = typename FutureBase::ValueType;
	using ExpectedType = typename FutureBase::ExpectedType;

	static_assert( !std::is_reference_v<Exec> );

public:
	BaseContinuableFuture( std::shared_ptr<SharedState<ValueType>> state, const Exec& exec )
		: FutureBase( std::move( state ) )
		, m_executor( exec )
	{}

	template <typename Function>
	auto Then( Function&& f ) &&
	{
		using R = ContinuationResultType_t<Function, ValueType>;

		auto[ future, promise ] = MakeFuturePromisePair<R>();
		this->m_state->SetContinuation( BoundContinuation( Qualifier{}, std::move( promise ), m_executor, std::forward<Function>( f ) ) );
		this->Discard();
		return std::move( future ).Via( m_executor );
	}

	template <typename Function>
	auto Chain( Function&& f ) &&
	{
		if constexpr ( IsFuture_v<ValueType> )
			return std::move( *this ).Unwrap().Via( m_executor ).Then( std::forward<Function>( f ) );
		else
			return std::move( *this ).Then( std::forward<Function>( f ) );
	}

	operator FutureBase() &&
	{
		return FutureBase( std::move( this->m_state ) );
	}

protected:
	Exec m_executor;
};

} // namespace Detail

template <typename T, typename Exec>
class ContinuableFuture : public Detail::BaseContinuableFuture<Future<T>, Exec, Detail::MoveQualifier>
{
public:
	using Detail::BaseContinuableFuture<Future<T>, Exec, Detail::MoveQualifier>::BaseContinuableFuture;

	ContinuableSharedFuture<T, Exec> Share();
};

template <typename T, typename Exec>
class ContinuableSharedFuture : public Detail::BaseContinuableFuture<SharedFuture<T>, Exec, Detail::ConstQualifier>
{
public:
	using Detail::BaseContinuableFuture<SharedFuture<T>, Exec, Detail::ConstQualifier>::BaseContinuableFuture;
};

template <typename T, typename Exec>
ContinuableSharedFuture<T, Exec> ContinuableFuture<T, Exec>::Share()
{
	return ContinuableSharedFuture<T, Exec>( std::move( this->m_state ), this->m_executor );
}

template <typename T>
template <typename Exec>
inline ContinuableFuture<T, Exec> Future<T>::Via( const Exec& executor ) &&
{
	return ContinuableFuture<T, Exec>( std::move( this->m_state ), executor );
}

template <typename T>
template <typename Exec>
inline ContinuableSharedFuture<T, Exec> SharedFuture<T>::Via( const Exec& executor ) &&
{
	return ContinuableSharedFuture<T, Exec>( std::move( this->m_state ), executor );
}

// Execution

template <typename Executor, typename Function, typename... Args>
inline auto TwoWayExecute( const Executor& exec, Function&& f, Args&&... args )
{
	using BindType = decltype( stdx::bind( std::forward<Function>( f ), std::forward<Args>( args )... ) );

	if constexpr ( HasTwoWayExecute_v<Executor, Function&&, Args&&...> )
	{
		return exec.TwoWayExecute( std::forward<Function>( f ), std::forward<Args>( args )... ).Via( exec );
	}
	else if constexpr ( HasTwoWayExecute_v<Executor, BindType> )
	{
		return exec.TwoWayExecute( stdx::bind( std::forward<Function>( f ), std::forward<Args>( args )... ) ).Via( exec );
	}
	else
	{
		using R = Detail::ContinuationResultType_t<BindType, void>;

		auto[ future, promise ] = MakeFuturePromisePair<R>();
		Execute( exec, Detail::Task( std::move( promise ), stdx::bind( std::forward<Function>( f ), std::forward<Args>( args )... ) ) );
		return std::move( future ).Via( exec );
	}
}

template <typename Executor, typename FutureType, typename Function, typename... Args>
inline auto ThenExecute( const Executor& exec, FutureType fut, Function&& f, Args&&... args )
{
	return std::move( fut ).Via( exec ).Then( stdx::bind( std::forward<Function>( f ), std::forward<Args>( args )... ) );
}

template <typename... Futures>
void WaitAll( Futures&... futures ) noexcept
{
	static_assert( std::conjunction_v<IsFuture<std::decay_t<Futures>>...> );
	( futures.Wait(), ... );
}

template <typename InputIt, std::enable_if_t<IsFuture_v<typename std::iterator_traits<InputIt>::value_type>, int> = 0>
void WaitAll( InputIt first, InputIt last ) noexcept
{
	static_assert( IsFuture_v<typename std::iterator_traits<InputIt>::value_type> );
	for ( ; first != last; ++first )
		first->Wait();
}

/*

template <typename... Futures>
using FutureTuple = std::tuple<FutureValueType_t<std::decay_t<Futures>>...>;

template <typename HeadFuture, typename... TailFutures, STDX_requires( IsFutureReference_v<HeadFuture> && std::conjunction_v<IsFutureReference<TailFutures>...> )
Future<FutureTuple<HeadFuture, TailFutures...>> WhenAll( HeadFuture&& headFuture, TailFutures&&... tailFutures )
{
	static_assert( sizeof...( TailFutures ) > 0 );
	static_assert( IsFutureRValue_v<HeadFuture&&> && std::conjunction_v<IsFutureRValue<TailFutures&&>...>, "futures must be moved into WhenAll()" );

	WhenAll( std::forward<TailFutures>( tailFutures )... ).Via( InlineExecutor() )
		.Then( [ headFuture = std::move( headFuture ) ]( FutureTuple<TailFutures...> values )
			{
				if constexpr ( IsVoidFuture_v<HeadFuture> )
					return headFuture.Via( InlineExecutor() ).Then( [ values = std::move( values ) ]()
						{
							return std::move( values );
						} );
				else
					return headFuture.Via( InlineExecutor() ).Then( [ values = std::move( values ) ]( FutureValueType_t<HeadFuture> value )
						{
							return std::tuple_cat( std::tie( std::move(  value ) ), std::move( values ) );
						} );
			} )
		.Unwrap();
}

template <typename HeadFuture, STDX_requires( IsFutureReference_v<HeadFuture&&> )
Future<FutureTuple<HeadFuture>> WhenAll( HeadFuture&& headFuture )
{
	static_assert( IsFutureRValue_v<HeadFuture&&>, "futures must be moved into WhenAll()" );
	if constexpr ( IsVoidFuture_v<HeadFuture> )
		return headFuture.Via( InlineExecutor() ).Then( []( FutureValueType_t<HeadFuture> value ) { return std::make_tuple( std::move( value ) ); } );
	else
		return headFuture.Via( InlineExecutor() ).Then( [] { return std::tuple<>(); } );
}

Future<std::tuple<>> WhenAll()
{
	return MakeReadyFuture<std::tuple<>>();
}

template <typename Container>
using FutureVector = std::vector<FutureValueType_t<typename stdx::container_traits<Container>::value_type>>;

template <typename Container>
Future<FutureVector<Container>> WhenAll( Container&& container )
{
	static_assert( std::is_rvalue_reference_v<Container>, "futures must be moved into WhenAll()" );

	if ( !std::empty( container ) )
	{
		using ValueType = FutureValueType_t<typename stdx::container_traits<Container>::value_type>;

		auto vectorFuture = std::move( *std::begin( container ) ).Via( InlineExecutor() )
			.Then( [ initSize = std::size( container ) ]( ValueType value )
				{
					FutureVector<Container> results;
					results.reserve( initSize );
					results.emplace( std::move( value ) );
					return results;
				} );

		for ( auto it = ++std::begin( container ), last = std::end( container ); it != last; ++it )
		{
			vectorFuture = std::move( vectorFuture )
				.Then( [ nextFuture = std::move( *it ) ]( FutureVector<Container> results )
					{
						return nextFuture.Via( InlineExecutor() ).Then( [ results = std::move( results ) ]( ValueType value ) mutable
							{
								results.emplace_back( std::move( value ) );
								return std::move( results );
							} );
					} )
				.Unwrap();
		}

		return vectorFuture;
	}
	else
	{
		return MakeReadyFuture<FutureVector<Container>>();
	}
}

*/

} // namespace Threading

namespace MT
{
	using namespace Threading;
}