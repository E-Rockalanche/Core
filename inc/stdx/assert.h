#pragma once

#ifdef _DEBUG

#include <cstdio>

#define dbLog( ... ) do{	\
	std::printf( __VA_ARGS__ );	\
	std::printf( "\n" );	\
} while( false )

#define dbLogToStdErr( ... ) std::fprintf( stderr, __VA_ARGS__ )

#define dbLogErrorLocation() dbLogToStdErr( "ERROR AT %s:%d:\n", __FILE__, __LINE__ )

#define dbLogError( ... ) do{	\
	dbLogErrorLocation();	\
	dbLogToStdErr(  __VA_ARGS__ );	\
	dbLogToStdErr( "\n" );	\
} while( false )

#define dbBreak() __debugbreak()

#define dbBreakMessage( ... ) do{\
	dbLog( __VA_ARGS__ );	\
	dbBreak();	\
} while(false)

#define dbAssertFail( message ) do{	\
		dbLogErrorLocation();	\
		dbLogToStdErr( "Assertion failed: " message );	\
} while(false)

#define dbAssert( condition ) do{	\
	if ( !( condition ) ) {	\
		dbAssertFail( #condition );	\
		dbBreak();	\
	}	\
} while( false )

#define dbAssertMessage( condition, ... ) do{	\
	if ( !( condition ) ) {	\
		dbAssertFail( #condition );	\
		dbLogToStdErr( __VA_ARGS__ );	\
		dbBreak();	\
	}	\
} while( false )

#define dbVerify( condition ) dbAssert( condition )

#define dbVerifyMessage( condition, ... ) dbAssertMessage( condition, __VA_ARGS__ )

// safe to use in constexpr function
#define dbExpects( condition )	\
	( ( condition ) ? (void)0 : (void)[]{ dbAssertFail( #condition ); } )

// safe to use in constexpr function
#define dbEnsures( condition )	\
	( ( condition ) ? (void)0 : (void)[]{ dbAssertFail( #condition ); } )

#else

#define dbLog( ... )
#define dbLogToStdErr( ... )
#define dbLogErrorLocation()
#define dbLogError( ... )
#define dbBreak()
#define dbBreakMessage( ... )
#define dbAssert( condition )
#define dbAssertMessage( condition, ... )
#define dbVerify( condition ) ( condition )
#define dbVerifyMessage( condition, ... ) ( condition )
#define dbExpects( condition )
#define dbEnsures( condition )

#endif