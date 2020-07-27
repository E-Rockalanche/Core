#pragma once

#if defined( _DEBUG ) || true

#include <cstdio>

#define MultiLineMacroBegin do{
#define MultiLineMacroEnd }while( false )

#define dbLog( ... ) do{	\
	std::printf( __VA_ARGS__ );	\
	std::printf( "\n" );	\
} while( false )

#define dbLogToStdErr( ... ) std::fprintf( stderr, __VA_ARGS__ )

#define dbLogErrorLocation() dbLogToStdErr( "ERROR AT %s:%d:\n", __FILE__, __LINE__ )

#define dbLogError( ... )	\
	MultiLineMacroBegin	\
	dbLogErrorLocation();	\
	dbLogToStdErr(  __VA_ARGS__ );	\
	dbLogToStdErr( "\n" );	\
	MultiLineMacroEnd

#define dbBreak() __debugbreak()

#define dbBreakMessage( ... )	\
	MultiLineMacroBegin	\
	dbLog( __VA_ARGS__ );	\
	dbBreak();	\
	MultiLineMacroEnd

#define dbAssertFail( message )	\
	MultiLineMacroBegin	\
		dbLogErrorLocation();	\
		dbLogToStdErr( "Assertion failed: " message );	\
	MultiLineMacroEnd

#define dbAssert( condition )	\
	MultiLineMacroBegin	\
	if ( !( condition ) ) {	\
		dbAssertFail( #condition );	\
		dbBreak();	\
	}	\
	MultiLineMacroEnd

#define dbAssertMessage( condition, ... )	\
	MultiLineMacroBegin	\
	if ( !( condition ) ) {	\
		dbAssertFail( #condition );	\
		dbLogToStdErr( __VA_ARGS__ );	\
		dbBreak();	\
	}	\
	MultiLineMacroEnd

#define dbVerify( condition ) dbAssert( condition )

#define dbVerifyMessage( condition, ... ) dbAssertMessage( condition, __VA_ARGS__ )

// safe to use in constexpr function
#define dbExpects( condition )	\
	MultiLineMacroBegin	\
	if ( !( condition ) ) {	\
		dbAssertFail( #condition );	\
		dbBreak();	\
	}	\
	MultiLineMacroEnd

#define dbEnsures( condition )	\
	MultiLineMacroBegin	\
	if ( !( condition ) ) {	\
		dbAssertFail( #condition );	\
		dbBreak();	\
	}	\
	MultiLineMacroEnd

#else

#define EmptyBlock do{}while(false)

#define dbLog( ... ) EmptyBlock
#define dbLogToStdErr( ... ) EmptyBlock
#define dbLogErrorLocation() EmptyBlock
#define dbLogError( ... ) EmptyBlock
#define dbBreak() EmptyBlock
#define dbBreakMessage( ... ) EmptyBlock
#define dbAssert( condition ) EmptyBlock
#define dbAssertMessage( condition, ... ) EmptyBlock
#define dbVerify( condition ) ( condition )
#define dbVerifyMessage( condition, ... ) ( condition )
#define dbExpects( condition ) EmptyBlock
#define dbEnsures( condition ) EmptyBlock

#endif