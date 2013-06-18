// BLuB
//	BleuLlama's microBasic
//
//	A basic-inspired, asm-ish programming language thing
//

////////////////////////////////////////////////////////////////////////////////
#ifdef DESKTOP
#include "ardsim.h"		// all of the arduino simulation libraries

#else

// arduino includes
#include <EEPROM.h>  		// EEProm support
#include <avr/pgmspace.h>	// PGMspace support for strings
#endif


////////////////////////////////////////////////////////////////////////////////

void setup()
{
	// set up serial port
	Serial.begin( 9600 );
	while( !Serial ) {
		; // wait for Leonardo to catch up
	}

	Serial.println( "BLuB Initialization:" );
} 


void loop()
{
	// print out something
	Serial.println( "Loop!" );

	// wait 1 second
	delay( 239 );

	Serial.print( "Time so far: " );
	Serial.println( millis() );
}
