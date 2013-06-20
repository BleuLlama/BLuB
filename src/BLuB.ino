// BLuB
//	BleuLlama's microBasic
//
//	A basic-inspired, asm-ish programming language thing
//

////////////////////////////////////////////////////////////////////////////////
// Version history

#define kBLuBVersion	"v0.02  2013-June-19  yorgle@gmail.com"

// v0.02  2013-June-19  New prompt (Smiley)
//			ops:	NP RE ST
//			cmds:   vars
//
// v0.01  2013-June-19  Line entry, line editing
//			cmds: 	help, mem, new, list
//			      	EEPROM: elist, eload, esave, enew
//				run, tron, troff
//
// v0.00  2013-June-18  Initial test versions



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

bool trace = false;

////////////////////////////////////////////////////////////////////////////////

#define kBufLen (16)
char buffer[kBufLen];

int latoi( char * buf )
{
	int v = 0;
	// find the integer starting at buf[0]
	while( *buf <= '9' && *buf >= '0' && v<kBufLen  ) {
		buffer[v++] = *buf;
		buf++;
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
		
#define SKIP_NUMBER( A ) \
	while(     (*A) >= '0' \
		&& (*A) <= '9' \
		&& (*A) != '\0' ) A++;

#define SKIP_WHITESPACE( A ) \
	while( (   (*A) == ' ' \
		|| (*A) == '\t' \
		|| (*A) == ',' \
	        ) && (*A) != '\0' ) A++;

////////////////////////////////////////////////////////////////////////////////

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
	Serial.println( "    help    mem     vars    new     list" );
	Serial.println( "    run     tron    troff" );
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

void cmd_vars( void )
{
	char buf[20];
	Serial.println( "Variables:" );
	for( int i=0 ; i<(kNVariables/2) ; i++ )
	{
		snprintf( buf, 20, "  %c %-9d", i+'a', variables[i] );
		Serial.print( buf );

		snprintf( buf, 20, "    %c %-9d", i+13+'a', variables[i+13] );
		Serial.println( buf );
	}
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
	Serial.println( "" );		// newline before
	Serial.print( programRam );	// dump it all out
	// no need for println, since program ram ends with newline
}


////////////////////////////////////////////////////////////////////////////////

void cmd_tron( void )
{
	trace = true;
	Serial.println( "Trace on." );
}

void cmd_troff( void )
{
	trace = false;
	Serial.println( "Trace off." );
}

char * findLine( int line )
{
	char *bufc = programRam;
	int cline = latoi( bufc );

	while( cline != line && *bufc != '\0' ) {
		// skip to next line
		while( *bufc != '\0' && *bufc != '\n' ) bufc++;
		// fill cline
		bufc++;
		cline = latoi( bufc );
	}

	return bufc;
}

#define kJRStop		(-3)
#define kJRNextLine	(-2)
#define kJRFirstLine	(-1)

int evaluate_line( char * line )
{
	int len = 0;
	char * buf = line;

	if( !line ) return kJRStop;

	// make sure there's an opcode
	while( *buf != '\0' && *buf != '\n' ) { len++; buf++; }
	if( len < 2 ) return kJRStop;

#define OpcodeIs( A, B )\
	(line[0] == (A) && line[1] == (B) )

	// the do-nothing ops
	if( OpcodeIs( 'N', 'P' )) return kJRNextLine;
	if( OpcodeIs( 'R', 'E' )) return kJRNextLine;

	// stop.
	if( OpcodeIs( 'S', 'T' )) return kJRStop;

	return kJRNextLine;
}

void cmd_run( void )
{
	int cline = 0;
	int next = kJRFirstLine;
	char *bufc = programRam;

	if( *bufc == '\0' ) return;

	while( *bufc && next != kJRStop ) {
		// determine the next line number
		if( next == kJRFirstLine ) {
			// starting, use the first one.
			bufc = programRam;

		} else if( next == kJRNextLine ) {
			// use the next one.
			while( *bufc != '\0' && *bufc != '\n' ) bufc++;
			bufc++;

		} else {
			// next contains the next line to execute
			// find the line
			bufc = findLine( next );
			if( *bufc == '\0' ) {
				Serial.print( (long) next, DEC );
				Serial.println( ": Line not found." );
				break;
			}
		}

		next = kJRNextLine; // set for next line

		if( *bufc == '\0' ) {
			// just in case.
			break;
		}

		// work on the line here
		char * ln = bufc;

		SKIP_NUMBER( ln );
		SKIP_WHITESPACE( ln );

		// trace output
		if( trace ) {
			Serial.print( "Line: " );
			char * tc = bufc;
			while( (*tc) != '\0' && (*tc) != '\n' ) {
				Serial.write( tc, 1 );
				tc++;
			}
			Serial.println( "" );


			// get user input (return)
			while( !Serial.available() );
			(void) Serial.read();
		}

		// do the thing!
		cline = latoi( bufc );
		next = evaluate_line( ln );
	}

	Serial.print( "Stopped at line " );
	Serial.println( (long)cline, DEC );
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

	// find the appropriate location to insert it

	int lineNo = latoi( theLine );

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
		if( cline > lineNo )
		{
			// shove the current data out to the end.
			int moveSz = strlen( bufc ) +1;
			char * srcpos = bufc + moveSz;
			char * dstpos = bufc + newSz + moveSz +1;

			do {
				*dstpos = *srcpos;
				srcpos--;
				dstpos--;
			} while( srcpos != bufc );
			*dstpos = *srcpos; // copy over the last one...

			// and insert the new content
			dstpos = bufc;
			srcpos = theLine;

			while( *srcpos != '\0' ) {
				*dstpos = *srcpos;
				dstpos++;
				srcpos++;
			}
			*dstpos = '\n';

			return;
		}

		// if not, move looking pointer to next line
		bufc = bufn;
		bufc++; // move past the newline
	}
	// shove the content out the length of the new string
	// copy the data in

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
	Serial.print( ":) " );
	
	// get a line of input
	getSerialLine( linebuf, kLineLen, false );

	// process it
	if( !strcmp( linebuf, "mem" )) { cmd_mem(); }

	else if( !strcmp( linebuf, "new" )) { cmd_new(); }
	else if( !strcmp( linebuf, "list" )) { cmd_list(); }
	else if( !strcmp( linebuf, "vars" )) { cmd_vars(); }
	else if( !strcmp( linebuf, "help" )) { cmd_help(); }

	else if( !strcmp( linebuf, "enew" )) { cmd_enew(); }
	else if( !strcmp( linebuf, "elist" )) { cmd_elist(); }
	else if( !strcmp( linebuf, "eload" )) { cmd_eload(); }
	else if( !strcmp( linebuf, "esave" )) { cmd_esave(); }

	else if( !strcmp( linebuf, "tron" )) { cmd_tron(); }
	else if( !strcmp( linebuf, "troff" )) { cmd_troff(); }
	else if( !strcmp( linebuf, "run" )) { cmd_run(); }

	else if( linebuf[0] >= '0' && linebuf[0] <= '9' ) {
		// it's starting with a number
		bptr = linebuf;

		// get the line number
		int v = latoi( bptr );
		if( v < 0 || v > 999999 ) {
			Serial.println( "Line Number out of range." );
			goto cleanup;
		}

		// skip the number and whitespace, to see if we're just 
		// entering the line, or there's more to it.

		SKIP_NUMBER( bptr );
		SKIP_WHITESPACE( bptr );
			
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
