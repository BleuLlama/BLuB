// BLuB
//	BleuLlama's microBasic
//
//	A basic-inspired, asm-ish programming language thing
//

////////////////////////////////////////////////////////////////////////////////
#ifdef DESKTOP
// desktop includes
#include <iostream>
#include <sys/time.h>
#include "eesim.h"

#else

// arduino includes
#include <EEPROM.h>  		// EEProm support
#include <avr/pgmspace.h>	// PGMspace support for strings
#endif


////////////////////////////////////////////////////////////////////////////////
// system macros...
#ifdef DESKTOP

// print hacks
#define PRINT( A ) 	std::cout << (A)
#define PRINTLN( A )	std::cout << (A) << std::endl;

// delay
#define delay( A )	usleep( (A) * 1000 )

// millis support
static timeval startTime;
void millis_start( void )
{
	gettimeofday( &startTime, NULL );
}

long millis( void )
{
	timeval endTime;
	long seconds, useconds;
	double duration;

	gettimeofday( &endTime, NULL );
	seconds = endTime.tv_sec - startTime.tv_sec;
	useconds = endTime.tv_usec - startTime.tv_usec;

	duration = seconds + useconds/1000000.0;

	return( (long) (duration * 1000) );
}


#else

#define PRINT( A ) 		Serial.print( A )
#define PRINTLN( A ) 		Serial.println( A )
		
#endif



////////////////////////////////////////////////////////////////////////////////

void setup()
{
#ifdef DESKTOP
	// set up millis
	millis_start();
#else
	// set up serial port
	Serial.begin( 9600 );
	while( !Serial ) {
		; // wait for Leonardo to catch up
	}
#endif

	PRINTLN( "BLuB Initialization:" );
} 


void loop()
{
	// print out something
	PRINTLN( "Loop!" );

	// wait 1 second
	delay( 239 );
	PRINT( "Time so far: " );
	PRINTLN( millis() );
}
