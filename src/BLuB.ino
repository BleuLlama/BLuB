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

#else
// arduino includes
#include <EEPROM.h>  /* NOTE: case sensitive */

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


////////////////////
// eeprom simulator

#define EE2END	(0x1000)
unsigned char _ee_[EE2END];

void ee_load( void )
{
	FILE * fp = fopen( "eeprom.dat", "r" );
	if( !fp ) return;
	fread( _ee_, EE2END, 1, fp );
	fclose( fp );
	PRINT( "Loaded EEprom" );
}

void ee_save( void )
{
	FILE * fp = fopen( "eeprom.dat", "w" );
	if( !fp ) return;
	fwrite(  _ee_, EE2END, 1, fp );
	fclose( fp );
	PRINT( "Saved EEprom" );
}


#define EE_READ( A ) 		( _ee_[ (A) % EE2END ] )
#define EE_WRITE( A, V ) \
	{ \
		_ee_[ (A) % EE2END ] = (V); \
		ee_save(); \
	}

#else
#define EE_READ( A )		( EEPROM.read( A ))
#define EE_WRITE( A, V ) 	( EEPROM.write( (A), (V) ))

#define PRINT( A ) 		Serial.print( A )
#define PRINTLN( A ) 		Serial.println( A )
		
#endif



////////////////////////////////////////////////////////////////////////////////

void setup()
{
#ifdef DESKTOP
	// set up millis
	millis_start();
	ee_load();
#else
	// set up serial port
	Serial.begin( 9600 );
	while( !Serial ) {
		; // wait for Leonardo to catch up
	}
#endif

	PRINTLN( "BLuB Initialization:" );
	long eesize = EE2END+1;
	PRINT( eesize );
	PRINTLN( " EEProm bytes available." );

	for( int i=0 ; i<10 ; i++ ) {
		PRINT( EE_READ( i ) );
		PRINT( ", " );
	}
	PRINTLN( "." );

	for( int i=0 ; i<10 ; i++ ) {
		EE_WRITE( i, i );
	}
} 


void loop()
{
	// print out something
	PRINTLN( "Loop!" );

	// wait 1 second
	delay( 239 );
	PRINT( "Time so far: " );
	PRINTLN( millis() );
#ifdef DESKTOP
	ee_save();
#endif
}
