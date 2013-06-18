// BLuB
//	BleuLlama's microBasic
//
//	A basic-inspired, asm-ish programming language thing
//

////////////////////////////////////////////////////////////////////////////////
// Version history
//
// v0.00  2013-June-18  Initial test versions

#define kBLuBVersion	"v0.00  2013-June-18  yorgle@gmail.com"


////////////////////////////////////////////////////////////////////////////////
#ifdef DESKTOP
#include "ardsim.h"		// all of the arduino simulation libraries

#else

// arduino includes
#include <EEPROM.h>  		// EEProm support
#include <avr/pgmspace.h>	// PGMspace support for strings
#endif


////////////////////////////////////////////////////////////////////////////////
#define kRamSize (E2END + 1 )

char programRam[ kRamSize ];


int ramFree = E2END;
int eeFree = E2END+1;


void cmd_mem( void )
{
	ramFree = strlen( (const char *)programRam );
	ramFree = kRamSize - ramFree;

	Serial.print( "    " );
	Serial.print( (long)ramFree, DEC );
	Serial.print( " of " );
	Serial.print( (long)kRamSize, DEC );
	Serial.println( " bytes free RAM" );



	// recompute eeFree
	int ch = 'X';

	// figure out how much is used
	for( eeFree=0 ; (eeFree<=E2END) && (ch != '\0') ; eeFree++ )
	{
		ch = EEPROM.read( eeFree );
		if( ch == '\0' ) continue;
	}
	eeFree = E2END+1 - eeFree; // turn it into free.


	Serial.print( "    " );
	Serial.print( (long)eeFree, DEC );
	Serial.print( " of " );
	Serial.print( (long)E2END+1, DEC );
	Serial.println( " bytes free EEProm" );
}

void cmd_help( void )
{
	Serial.println( "BLuB Interface" );
	Serial.println( kBLuBVersion );
	cmd_mem();

	Serial.println( "" );
	Serial.println( "Available commands:" );
	//                   ------- ------- ------- ------- -------
	Serial.println( "    help    mem" );
	Serial.println( "    elist   eload   esave   eformat" );
	//                   ------- ------- ------- ------- -------
}


////////////////////////////////////////////////////////////////////////////////
void setup()
{
	// set up serial port
	Serial.begin( 9600 );
	while( !Serial ) {
		; // wait for Leonardo to catch up
	}

	Serial.println( "BLuB Interface" );
	Serial.println( kBLuBVersion );
	cmd_mem();
} 



// getSerialLine
//   gets a line (terminated by \n newline) from the serial port
//   stores it in the buffer pointed to by "buf"
//   "buf" is a maximum of "maxbuf" bytes
//   if echoback is true, it will also print stuff out as it gets it
void getSerialLine( char * buf, int maxbuf, boolean echoback )
{
  int chp = 0;

  // read a line or so into our buffer
  do {
    buf[chp] = Serial.read();
    if( echoback ) Serial.write( buf[chp]);
    chp++;
  } while(    Serial.available()
           && buf[chp-1] != '\n'
           && chp < maxbuf );

  // terminate the buffer
  buf[chp] = '\0';

  // strip newline
  if( chp > 0 && buf[ chp-1 ] == '\n' ) buf[chp-1] = '\0';

  if( echoback ) Serial.println( "" );
}


#define kLineLen (32)
char linebuf[kLineLen];

void cmd_eformat( void )
{
  Serial.print( "Formatting EEPROM..." );
  for( int i=0 ; i<=E2END ; i++ )
  {
    EEPROM.write( i, 0x00 );
//    digitalWrite( kLED, i & 0x020 );
    if( i%64 == 0 ) Serial.print( "." );
  }
  Serial.println( "...DONE!" );
}

void cmd_elist( void )
{
  int ch = 'X';

  for( int i=0 ; (i<=E2END) && (ch != '\0') ; i++ )
  {
    ch = EEPROM.read( i );

    if( ch == '\0' ) continue;
    Serial.write( (char *) &ch, 1 );
  }
}

void cmd_eload( void )
{
}

void cmd_esave( void )
{
}


void loop()
{
	// print out something
	Serial.print( "> " );
	
	// get a line of input
	getSerialLine( linebuf, kLineLen, false );

	// process it
	if( !strcmp( linebuf, "mem" )) { cmd_mem(); }
	else if( !strcmp( linebuf, "eformat" )) { cmd_eformat(); }
	else if( !strcmp( linebuf, "elist" )) { cmd_elist(); }
	else if( !strcmp( linebuf, "eload" )) { cmd_eload(); }
	else if( !strcmp( linebuf, "esave" )) { cmd_esave(); }
	else if( linebuf[0] != '\0' ){
		Serial.println( "Huh?" );
	}

	Serial.println( "" );
}
