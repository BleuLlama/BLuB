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
#include "sersim.h"
#include "miscsim.h"

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
