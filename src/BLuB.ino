// BLuB
//	BleuLlama's microBasic
//
//	A basic-inspired, asm-ish programming language thing
//

////////////////////////////////////////////////////////////////////////////////
// system includes...
#ifdef DESKTOP
#include <iostream>
#endif


////////////////////////////////////////////////////////////////////////////////
// system macros...
#ifdef DESKTOP

// print with no newline
#define PRINT( A ) \
	std::cout << (A)
	
// print with newline
#define PRINTLN( A ) \
	std::cout << (A) << std::endl;

// delay for milliseconds
#define DELAY( A ) \
	usleep( (A) * 1000 )

#else

#define PRINT ( A ) \
	Serial.print( A )

#define PRINTLN( A ) \
	Serial.println( A )

#define DELAY( A ) \
	delay( A )
#endif

////////////////////////////////////////////////////////////////////////////////

void setup()
{
#ifdef DESKTOP
#else
	Serial.begin( 9600 );
	while( !Serial ) {
		; // wait for Leonardo to catch up
	}
#endif

	PRINT( "Starting..." );
} 


void loop()
{
	// print out something
	PRINT( "Loop!" );

	// wait 1 second
	DELAY( 1000 );
}
