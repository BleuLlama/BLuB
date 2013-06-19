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
#define kEESize  (E2END + 1 )

#define kNVariables (26)

char programRam[ kRamSize ];
int variables[ kNVariables ];

int ramFree = 0;
int eeFree = 0;


void cmd_mem( void )
{
	ramFree = strlen( (const char *)programRam );
	ramFree +=1;
	ramFree = kRamSize - ramFree;

	Serial.print( "    " );
	Serial.print( (long)ramFree, DEC );
	Serial.print( " of " );
	Serial.print( (long)kRamSize, DEC );
	Serial.println( " bytes free RAM" );

	// recompute eeFree
	int ch = 'X';

	// figure out how much is used
	for( eeFree=0 ; (eeFree<=kEESize) && (ch != '\0') ; eeFree++ )
	{
		ch = EEPROM.read( eeFree );
		if( ch == '\0' ) continue;
	}
	eeFree = kEESize - eeFree; // turn it into free.


	Serial.print( "    " );
	Serial.print( (long)eeFree, DEC );
	Serial.print( " of " );
	Serial.print( (long)kEESize, DEC );
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
	Serial.println( "    help    mem     new     list    run" );
	Serial.println( "    elist   eload   esave   enew" );
	//                   ------- ------- ------- ------- -------
}


////////////////////////////////////////////////////////////////////////////////


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



void cmd_enew( void )
{
	Serial.print( "Formatting EEPROM " );
	for( int i=0 ; i<kEESize ; i++ )
	{
		EEPROM.write( i, 0x00 );
		//    digitalWrite( kLED, i & 0x020 );
		if( i%64 == 0 ) Serial.print( "." );
	}
	Serial.println( " Done." );
}

void cmd_elist( void )
{
	int ch = 'X';

	for( int i=0 ; (i<kEESize) && (ch != '\0') ; i++ )
	{
		ch = EEPROM.read( i );

		if( ch == '\0' ) continue;
		Serial.write( (char *) &ch, 1 );
	}
}

void cmd_eload( void )
{
	int i;
	int ch = 'X';

	for( i=0 ; (i<kEESize) && (ch != '\0') ; i++ )
	{
		ch = EEPROM.read( i );

		programRam[i] = ch;
		if( ch == '\0' ) continue;
	}

	Serial.print( (long)i, DEC );
	Serial.println( " bytes loaded from EEPROM." );
}

void cmd_esave( void )
{
	int i;
	int ch = 'X';

	for( i=0 ; (i<kEESize) && (ch != '\0') ; i++ )
	{
		ch = programRam[i];
		EEPROM.write( i, ch );

		if( ch == '\0' ) continue;
	}

	Serial.print( (long)i, DEC );
	Serial.println( " bytes saved to EEPROM." );
}

#define VarCharToIndex( A )\
		( (A) - 'a' )

void init_vars( void )
{
	for( int i=0 ; i<kNVariables ; i++ )
	{
		variables[i] = 0;
	}

	// (T)rue
	variables[ VarCharToIndex( 't' ) ] = 0xff;

	// (H)alf
	variables[ VarCharToIndex( 'h' ) ] = 0x80;

	// (F)alse, (Z)ero
	variables[ VarCharToIndex( 'f' ) ] = 0x00;
	variables[ VarCharToIndex( 'z' ) ] = 0x00;
}

void cmd_new( void )
{
	for( int i=0 ; i<kRamSize ; i++ )
	{
		programRam[ i ] = '\0';
	}
	init_vars();

}

void cmd_list( void )
{
	Serial.println( programRam );
}

void cmd_run( void )
{
	Serial.println( "Not yet." );
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
	cmd_new();
	init_vars();
	cmd_mem();
} 

#define kBufLen (16)
char buffer[16];

int latoi( char * buf )
{
	int v = 0;
	// find the integer starting at buf[0]
	while( *buf <= '9' && *buf >= '0' && v<kBufLen  ) {
		buffer[v++] = *buf;
		*buf++;
	}
	if( v == 0 ) {
		// no number!
		return -1;
	}

	buffer[v] = '\0';

	// my atoi (save on library overhead)
	v = 0;
	buf = buffer;
	while( *buf ) {
		v *= 10;
		v += (*buf)-'0';
		buf++;
	}
	return v;
}

void cmd_removeLine( int lineNo, bool verbose )
{
	char *bufc = programRam;
	char *bufn = 0;

	while( *bufc ) {
		// if we're looking at '\0', error line not found, return
	    	if( *bufc == '\0' ) break;

		// check the current pointer for the matching line
		int cline = latoi( bufc );

		// find the end point
		bufn = bufc;
		while( *bufn != '\0' && *bufn != '\n' ) bufn++;

		// if it matches, 
		if( cline == lineNo )
		{
		    	// scootch endpoint to end of ram down to cpos
			memcpy( bufc, bufn+1, strlen( bufn ) );
			return;
		}

		// if not, move looking pointer to next line
		bufc = bufn;
		bufc++; // move past the newline
	}

	if( verbose )
		Serial.print( "Error: Line not found." );
}

void cmd_insertLine( char * theLine )
{
	// for now, we'll just shove it into the end of program ram
	// char programRam[ kRamSize ];

	long pgmSz = strlen( programRam );
	long newSz = strlen( theLine );

	if( (pgmSz + newSz) >= kRamSize )
	{
		Serial.println( "Out of memory!" );
		return;
	}

	// append it on the end for now
	strcat( programRam, theLine );
	strcat( programRam, "\n" );	// new line

	// parsing happens at runtime. You can store whatever.
}


#define kLineLen (80)
char linebuf[kLineLen];
char * bptr;

void loop()
{
	// print out something
	Serial.print( "> " );
	
	// get a line of input
	getSerialLine( linebuf, kLineLen, false );

	// process it
	if( !strcmp( linebuf, "mem" )) { cmd_mem(); }

	else if( !strcmp( linebuf, "new" )) { cmd_new(); }
	else if( !strcmp( linebuf, "list" )) { cmd_list(); }

	else if( !strcmp( linebuf, "enew" )) { cmd_enew(); }
	else if( !strcmp( linebuf, "elist" )) { cmd_elist(); }
	else if( !strcmp( linebuf, "eload" )) { cmd_eload(); }
	else if( !strcmp( linebuf, "esave" )) { cmd_esave(); }

	else if( !strcmp( linebuf, "run" )) { cmd_run(); }

	else if( linebuf[0] >= '0' && linebuf[0] <= '9' ) {
		bptr = linebuf;

		// get the line number
		int v = latoi( bptr );
		if( v < 0 || v > 999999 ) {
			Serial.println( "Line Number out of range." );
			goto cleanup;
		}

		// skip the number and whitespace, to see if we're just 
		// entering the line, or there's more to it.

		// skip number
		while(     (*bptr) >= '0'
			&& (*bptr) <= '9'
			&& (*bptr) != '\0' ) bptr++;

		// skip whitespace
		while( (   (*bptr) == ' '
			|| (*bptr) == '\t'
			|| (*bptr) == ','
		       ) && (*bptr) != '\0' ) bptr++;
			
		if( *bptr == '\0' ) {
			cmd_removeLine( v, true );	// just remove it
		} else {
			cmd_removeLine( v, false );	// remove it first
			cmd_insertLine( linebuf );
		}

	}
	else if( linebuf[0] != '\0' ){
		Serial.println( "Huh?" );
	}

cleanup:
	Serial.println( "" );
}
