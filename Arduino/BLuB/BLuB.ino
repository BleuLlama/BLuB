// BLuB
//	BleuLlama's microBasic
//
//	A basic-inspired, asm-ish programming language thing
//

////////////////////////////////////////////////////////////////////////////////
// Version history

#define kBLuBVersion	"v1.08  2013-Aug-01  yorgle@gmail.com"

// v1.08  2013-Aug-01   Mass Storage beginnings
//			delay() after newline streamlined into the macros
//
#define kIncludeMassStorage

// v1.07  2013-July-26  "clear" command
//			variables 'e' and 'r' are preset with EE/RAM sizes
//
// v1.06  2013-July-19  Unformatted EEprom display for "mem" and "elist"
//
// v1.05  2013-July-16  PC op added to print out a value as a character
//			Bug fix for EO/PO indexing/wraparound
//
// v1.04  2013-July-06  Serial Delay for \n is set in the EEPROM config byte
//
// v1.03  2013-July-05  SerialprintNewline added to save a few bytes
//			response and error strings adjusted to save bytes
//			9600 baud by default
//			FASTSERIAL and kSerialLineDelay added for slow terms
//			EO and PO now can have multiple items per line
//			eg:  EO 509 66 67 1
//
// v1.02  2013-July-03  (abandoned) 
//			(reworking of the main if loop to save program space.)
//
// v1.01  2013-June-29  MS added
//
// v1.00  2013-June-27  Version bump to 1.0!
//			pulled out unneeded DESKTOP code.
//
// v0.08  2013-June-26  Autoload and Autorun added
//			ctrl-c/ctrl-d/Z to break a running program
//
// v0.07  2013-June-25  PEek, POke, for RAM and EEPROM implemented
//			auto-pinMode()
//			4800 baud
//
// v0.06  2013-June-24  Fixes for Arduino-builds
//			PROGMEM for low memory usage (ATMega 168)
//			Opcode parsing: uppercase are skipped, eg PR or PRINT are OK
//
// v0.05  2013-June-24  rearranged opcode names, added IC/L
//			better documentation
//			added pre-text whitespace elimination
//			WA, RN, RA, AS
//			PRint changed from PP
//			CAll added (GOSUB)
//			IF statement added
//			ON statement added
//
// v0.04  2013-June-21  cmds: files,load,save then removed. oops
//			ops	G
//
// v0.03  2013-June-21  ops:	all but Gosubs
//			pointer bugfixes
//
// v0.02  2013-June-19  New prompt (Smiley)
//			ops:	NP, RE, ST
//				LD(!)
//				PP, PL
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
// Desktop-based includes
#include "ardsim.h"		// all of the arduino simulation libraries

// Desktop defines
#undef kLocalEcho

#else
////////////////////////////////////////
// Arduino includes
#include <EEPROM.h>  		// EEProm support
#include <avr/pgmspace.h>	// PGMspace support for strings

// Arduino Defines
#define kLocalEcho (1)

// Reclaim more RAM: http://www.adafruit.com/blog/2008/04/17/free-up-some-arduino-sram/
#endif


// Autorun!
//	to use this, set the following bytes:
// 		E2END-2 to 'B'
//		E2END-1 to 'L'
//		E2END   to 0x01  (autorun flag)
//	other bits may have future uses.

#define kFlagAutorun (0x01)

// when these are set in the config byte, the associated values are added to the newline delay 
// (added for slow displays with no flow control)
#define kFlagDelay8 (0x80)    /* (128) 500 ms delay per line */
#define kFlagDelay4 (0x40)    /* (64)  250 ms delay */
#define kFlagDelay2 (0x20)    /* (32)  50 ms delay */

int serialDelay  = 0;

// Serial baud rate
#define kSerialBaud  (9600)

////////////////////////////////////////
// Common

#define kEESize  (E2END + 1 )
#define kRamSize ((RAMEND + 1 ) - 920)

char programRam[ kRamSize ];

#define kNVariables (26)
int variables[ kNVariables ];

// display runtime trace information
bool trace = false;

// Parameters for CAll/gosub
#define kNGosubs (8)
int gosubLevel = 0;
char * gosubStack[ kNGosubs ];

// Return values for the opcode handlers
#define kJRInterrupt    (-7)    /* ctrl-c/break interrupted */
#define kJRGosubStack	(-6)	/* Stack error for CAll */
#define kJRDBZError	(-5)	/* Divide by Zero error */
#define kJRSyntaxError	(-4)	/* general syntax erorr */
#define kJRStop		(-3)	/* runtime should stop */
#define kJRNextLine	(-2)	/* runtime should advance to the next line (NOERROR) */
#define kJRFirstLine	(-1)	/* runtime should start at the first line */
/* note: 0..Positive numbers are line numbers */

////////////////////////////////////////////////////////////////////////////////
// Support for PROGMEM stuff

#ifdef ARDUINO
// macros to help in doing the PROGMEM string manipulations and displays

// replace Serial.print("string") with SerialPrint("string")
#define Serialprint(x) SerialPrint_P(PSTR(x))
void SerialPrint_P(PGM_P str) {
  for (uint8_t c; (c = pgm_read_byte(str)); str++) Serial.write(c);
}

#define Serialprintln( x ) \
        Serialprint( x ); \
        Serialprint( "\n" )  /* FUTURE should we be \n\r? */ \
	delay( serialDelay );

#define SerialprintNewline() \
	Serial.println();

#else
////////////////////////////////////////
// Not arduino
#define Serialprint( x )     Serial.print( x )
#define Serialprintln( x )   Serial.println( x ); \
	delay( serialDelay )
#define SerialprintNewline() Serialprintln( "" )

#endif


////////////////////////////////////////////////////////////////////////////////

void SetSerialDelay( void )
{
	int ch;

	// check for sentinel
	ch = EEPROM.read( E2END-2 );
	if( ch != 'B' ) return;
	ch = EEPROM.read( E2END-1 );
	if( ch != 'L' ) return;

	ch = EEPROM.read( E2END );

        serialDelay = 0;
        if( ch & kFlagDelay2 ) serialDelay += 50;
        if( ch & kFlagDelay4 ) serialDelay += 250;
        if( ch & kFlagDelay8 ) serialDelay += 500;        
}


////////////////////////////////////////////////////////////////////////////////

// buffer is used for keyboard input, as well as string manipulation routines
#define kBufLen (32)
char buffer[kBufLen];

// PRINT_LINE for debugging
#define PRINT_LINE( L ) \
	{ \
		for( char * tl = (L) ; *tl != '\n' && *tl != '\0' ; tl++ ) \
			Serial.write( tl, 1 ); \
		Serial.println( "" ); \
	}

////////////////////////////////////////////////////////////
// Skip Number

#define macroSKIP_NUMBER( A ) \
	while(     (*A) >= '0' \
		&& (*A) <= '9' \
		&& (*A) != '\0' ) (A)++;

void SkipNumber( char ** l )
{
	macroSKIP_NUMBER( *l );
}
#define SKIP_NUMBER( A ) \
	SkipNumber( &A )


////////////////////////////////////////////////////////////
// Skip Whitespace

#define macroSKIP_WHITESPACE( A ) \
	while( (   (*A) == ' ' \
		|| (*A) == '\t' \
		|| (*A) == ',' \
	        ) && (*A) != '\0' ) (A)++; /* parens here are IMPORTANT */

void SkipWhitespace( char ** l )
{
	macroSKIP_WHITESPACE( *l );
}

#define SKIP_WHITESPACE( A ) \
	SkipWhitespace( &A )

////////////////////////////////////////////////////////////

#define macroSKIP_UPPERCASE( x ) \
	while(     (*x) >= 'A' \
		&& (*x) <= 'Z' \
		&& (*x) != '\0' ) (x)++;

void SkipUppercase( char ** l )
{
	macroSKIP_UPPERCASE( *l );
}

#define SKIP_UPPERCASE( A ) \
	SkipUppercase( &A )
////////////////////////////////////////////////////////////

int myAtoi( char * buf )
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


// same as the above, but it affects the line pointer
int myAtoiP( char ** buf, int * next )
{
	if( *next == kJRSyntaxError ) return 0;
	int v = myAtoi( *buf );
	SKIP_NUMBER( *buf );
	return v;
}


////////////////////////////////////////////////////////////////////////////////

void cmd_mem( void )
{
	int rfree = strlen( (const char *)programRam );
	rfree +=1;
	rfree = kRamSize +1 - rfree;

        SetSerialDelay();

	Serialprint( "Free:\n   " );
	Serial.print( (long)rfree, DEC );
	Serialprint( " / " );
	Serial.print( (long)kRamSize, DEC );
	Serialprintln( " RAM" );

	Serialprint( "   " );

	// see if EEPROM is in use
	int ch = EEPROM.read( 0 );

	if( ch == 0x0ff ) {
		Serialprint( "Unformatted" );
	} else {
        
        	// compute EEPROM free space
        
        	// figure out how much is used
        	for( rfree=0 ; (rfree<=kEESize) && (ch != '\0') ; rfree++ )
        	{
        		ch = EEPROM.read( rfree );
        		if( ch == '\0' ) continue;
        	}
        	rfree = kEESize - rfree; // turn it into free.
    	        Serial.print( (long)rfree, DEC );
        }
	Serialprint( " / " );
	Serial.print( (long)kEESize, DEC );
	Serialprintln( " EEPROM" );
}

void cmd_help( void )
{
        SetSerialDelay();
        
	Serialprintln( "BLuB " kBLuBVersion );
	cmd_mem();

#ifdef DESKTOP
	Serialprintln( "\nCommands:" );
	//                   ------- ------- ------- ------- -------
	Serialprintln( "    help    mem     vars    new     list" );
	Serialprintln( "    run     tron    troff" );
	Serialprintln( "    elist   eload   esave   enew" );
	Serialprintln( "    msbanks msload  mssave  msnew" );
	//                   ------- ------- ------- ------- -------
#endif
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
    while( !Serial.available() );
    buf[chp] = Serial.read();
    if( echoback ) Serial.write( buf[chp] );
    chp++;
  } while(    //Serial.available()
           buf[chp-1] != '\n'
           && buf[chp-1] != '\r'
           && chp < maxbuf );

  // terminate the buffer
  buf[chp] = '\0';

  // strip newline
  if( chp > 0 && ( buf[ chp-1 ] == '\n' || buf[ chp-1 ] == '\r') ) { buf[chp-1] = '\0'; chp--; }
  if( chp > 0 && ( buf[ chp-1 ] == '\n' || buf[ chp-1 ] == '\r') ) buf[chp-1] = '\0';

  if( echoback ) Serialprint( "\n" );
}



void cmd_enew( void )
{
	Serialprint( "Formatting " );
	for( int i=0 ; i<kEESize ; i++ )
	{
		EEPROM.write( i, 0x00 );
		//    digitalWrite( kLED, i & 0x020 );
		if( i%64 == 0 ) Serialprint( "." );
	}
	Serialprintln( "\nDone." );
}

void cmd_elist( void )
{
	int ch = EEPROM.read( 0 );

        SetSerialDelay();
        
        if( ch == 255 ) {
          Serialprintln( "Unformatted" );
          return;
        }

	for( int i=0 ; (i<kEESize) && (ch != '\0') ; i++ )
	{
		ch = EEPROM.read( i );

		if( ch == '\n' ) delay( serialDelay );

		if( ch == '\0' ) continue;
		Serial.write( (const uint8_t *) &ch, 1 );
	}
}

void cmd_eload( void )
{
	int i = 0;
	int ch = EEPROM.read( 0 );

        if( ch == 255 ) {
          Serialprintln( "Unformatted" );
          return;
        }

	for( i=0 ; (i<kEESize) && (ch != '\0') ; i++ )
	{
		ch = EEPROM.read( i );

		programRam[i] = ch;
		if( ch == '\0' ) continue;
	}

	Serial.print( (long)i, DEC );
	Serialprintln( " bytes loaded." );
}

void cmd_esave( void )
{
	int i = 0;
	int ch = 'X';

	for( i=0 ; (i<kEESize) && (ch != '\0') ; i++ )
	{
		ch = programRam[i];
		EEPROM.write( i, ch );

		if( ch == '\0' ) continue;
	}

	Serial.print( (long)i, DEC );
	Serialprintln( " bytes saved." );
}


////////////////////////////////////////////////////////////////////////////////
// Mass storage (files on desktop, banks of SD on device)
#ifdef kIncludeMassStorage

// ms format:
//	bank 0: reserved
//	bank 1..N: program slots:
//		   just contain the program

#ifdef DESKTOP
#define kMSSize		(1024 * 1024 * 8) /* 8 meg mass size */
#define kMSBlockSize	(512)
#endif


void cmd_msnew( void )
{
	Serialprintln( "NEW." );
#ifdef DESKTOP
	MASSSTORAGE.Open();
	MASSSTORAGE.Format();
	MASSSTORAGE.Close();
#endif
}

void cmd_msbanks( void )
{
	int done = 0;
	int bank = 0;
	int b = 0;
	char ch;
	Serialprintln( "BANKS:" );

#ifdef DESKTOP
	MASSSTORAGE.Open();
	ch = MASSSTORAGE.read( 0, 0 ); /* bank, byte */
	if( ch != 'B' ) {
		Serialprintln( "Not formatted." );
		MASSSTORAGE.Close();
		return;
	}
	while( done == 0 && bank < 100 ) {
		ch = MASSSTORAGE.read( bank, 0 );
		if( ch == '\0' ) {
			//Serialprint( "End." );
			//done = 1;
		} else {
			Serial.print( (long)bank, DEC );
			Serialprint( ": " );

			b = 0;
			ch = MASSSTORAGE.read( bank, b );
			while( ch != '\0' && ch != '\n' ) {
				Serial.write( ch );
				b++;
				ch = MASSSTORAGE.read( bank, b );
			}
			SerialprintNewline();
		}
		bank++;
	}
	MASSSTORAGE.Close();
#endif
}

void cmd_mssave( char * line )
{
	if( *line == '\0' ) {
		Serialprintln( "MSSAVE: No bank!" );
		return;
	}

	int bank = myAtoi( line );

	if( bank < 0 || bank > 255 ) {
		Serialprintln( "MSSAVE: Bad bank!" );
		return;
	}


	MASSSTORAGE.Open();
	MASSSTORAGE.erase( bank );
	int pos = 0;
	while( programRam[pos] != '\0' )
	{
		MASSSTORAGE.write( bank, pos, programRam[pos] );
		pos++;
	}
	MASSSTORAGE.write( bank, pos, '\0' );
	MASSSTORAGE.Close();

	Serialprint( "SAVED TO " );
	Serial.print( (long)bank, DEC );
	SerialprintNewline();
}

void cmd_msload( char * line )
{
	if( *line == '\0' ) {
		Serialprintln( "MSLOAD: No bank!" );
		return;
	}

	int bank = myAtoi( line );

	if( bank < 0 || bank > 255 ) {
		Serialprintln( "MSLOAD: Bad bank!" );
		return;
	}

	MASSSTORAGE.Open();
	int pos = 0;
	programRam[pos] = MASSSTORAGE.read( bank, pos );
	while( programRam[pos] != '\0' )
	{
		pos++;
		programRam[pos] = MASSSTORAGE.read( bank, pos );
	}
	MASSSTORAGE.Close();

	Serialprint( "LOADED FROM " );
	Serial.print( (long)bank, DEC );
	SerialprintNewline();
}

#endif



////////////////////////////////////////////////////////////////////////////////
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

	// (E)Eprom size and (R)AM size 
	variables[ VarCharToIndex( 'e' ) ] = kEESize;
	variables[ VarCharToIndex( 'r' ) ] = kRamSize;
	
}


void cmd_vars( void )
{
	Serialprintln( "Vars:" );
	for( int i=0 ; i<(kNVariables/2) ; i++ )
	{
		snprintf( buffer, kBufLen, "  %c: %-5d", i+'a', variables[i] );
		Serial.print( buffer );

		snprintf( buffer, kBufLen, " %c: %-5d", i+13+'a', variables[i+13] );
		Serial.println( buffer );
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
#ifdef NEVER
	SerialprintNewline();		// newline before
	Serial.print( programRam );	// dump it all out
	// no need for println, since program ram ends with newline
#else
        SetSerialDelay();
        
	// SLOW serial - add delays per line to keep up with my slow display
	int p = 0;
	while( programRam[p] != '\0' && p < kRamSize )
	{
		Serial.write( programRam[p] );
		if( programRam[p] == '\n'  ) delay( serialDelay );
		p++;
	}
#endif
}


////////////////////////////////////////////////////////////////////////////////


char * findLine( int line )
{
	char *bufc = programRam;
	int cline = myAtoi( bufc );

	while( cline != line && *bufc != '\0' ) {
		// skip to next line
		while( *bufc != '\0' && *bufc != '\n' ) bufc++;
		// fill cline
		bufc++;
		cline = myAtoi( bufc );
	}

	return bufc;
}



// parameters can be:
//	a-z 	variable source or destination
//	0..9 	integer source
//	"x"	text string (for printing)

// 
int getValue( char * line )
{
	if( !line ) return 0;

	SKIP_WHITESPACE( line );

	// check for variable
	if( *line >= 'a' && *line <= 'z' ){
		// it's a variable. Dereference it.
		return variables[VarCharToIndex( *line )];
	}

	// check for number
	if( *line >= '0' && *line <= '9' ) {
		return myAtoi( line );
	}
	
	// error!
	return -9999;
}


#define OpcodeIs( A, B )	(op0 == (A) && op1 == (B) )

#define isVarName( L )		( (L)>='a' && (L)<='z' )

void storeVariable( int identifier, int data, int next )
{
	if( identifier < 'a' || identifier > 'z' ) return;
	if( next == kJRSyntaxError ) return;
	variables[VarCharToIndex( identifier )] = data;
}

int retrieveVariable( int identifier )
{
	if( identifier < 'a' || identifier > 'z' ) return 0;
	return variables[VarCharToIndex( identifier )];
}


char getDestVarname( char ** line, int * next )
{
	if( !line || !next ) return '0';

	if( !isVarName( **line )) {
		*next = kJRSyntaxError;
		return '0';
	}

	// increment past it for the next calls
	char vn = **line;
	(*line)++;

	return vn;
}

int getParamValue( char ** line, int * next )
{
	if( !line || !next ) return 9999;
	if( *next == kJRSyntaxError ) return 8888;

	SKIP_WHITESPACE( *line );

	// check for variable
	if( **line >= 'a' && **line <= 'z' ){
		// it's a variable. Dereference it.
		(*line)++;
		return variables[VarCharToIndex( *((*line)-1) )];
	}


	// check for number
	if( **line >= '0' && **line <= '9' ) {
		return myAtoiP( line, next );
	}
	
	// error!
	*next = kJRSyntaxError;
	return 0;
}

#define kGoCa_unk	(0)
#define kGoCa_GO	(1)
#define kGoCa_CA	(2)

char getGoOrCa( char ** line, int * next )
{
	SKIP_WHITESPACE( *line );
	char opY = **line; (*line)++;
	char opZ = **line; (*line)++;

	if( opY == 'G' && opZ == 'O' ) return kGoCa_GO;
	if( opY == 'C' && opZ == 'A' ) return kGoCa_CA;
	return kGoCa_unk;
}

void doCall( int newLine, char **bufc, int * next )
{
	if( *next == kJRSyntaxError ) return;

	if( gosubLevel < kNGosubs ) {
		*next = newLine;
		gosubStack[ gosubLevel ] = *bufc;
		gosubLevel++;
	} else {
		*next = kJRGosubStack;
	}
}

int evaluate_line( char * line, char **bufc )
{
	int len = 0;
	char * buf = line;
	char varname = 0;
	int valueA = 0;
	int valueB = 0;
	int valueC = 0;
	int next = kJRNextLine;

	char op0, op1;

	if( !line ) return kJRStop;

	// make sure there's an opcode
	while( *buf != '\0' && *buf != '\n' ) { len++; buf++; }
	if( len < 2 ) return kJRStop;

	// store the opcode, and increment past it.
	op0 = *line++;
	op1 = *line++;

	// skip past any decorative uppercase
	SKIP_UPPERCASE( line );

	// skip any pre-pended whitespace
	SKIP_WHITESPACE( line );

	////////////////////////////////////////
	// STRUCTURE

	// RE - REM - comment
	if( OpcodeIs( 'R', 'E' )) return kJRNextLine;

	// EN - END - stop runtime
	if( OpcodeIs( 'E', 'N' )) return kJRStop;



	////////////////////////////////////////
	// USER IO

	// PR - PRINT ; - print out a variable, parameter or string
	// PC - PRINT ; - print out a character parameter CHR$
	// PL - PRINT - print out a variable, parameter or string, with newline
	if(    OpcodeIs( 'P', 'R' )
	    || OpcodeIs( 'P', 'C' )
	    || OpcodeIs( 'P', 'L' ) ) {

		if( *line == '\0' || *line == '\n' ) {
			// nothing.
			// this lets people do PL for newlines

		} else if( *line == '"' ) {
			// printout the literal string
			line++;
			
			while( *line != '"' ) {
				Serial.write( (const uint8_t *)line, 1 );
				line++;
			}
		} else {
			valueA = getValue( line );

			if( op1 == 'C' ) {
				Serial.write( valueA );
				Serial.flush();
			} else {
				Serial.print( (long) valueA, DEC );
			}
		}
		if( op1 == 'L' ) {
			SerialprintNewline();
		}

		return kJRNextLine;
	}


	// IC - INCHR - input a single character to a variable
	// IL - INCHR (to EOL) - absorb to the end of the line, return first 
	if(    OpcodeIs( 'I', 'C' )
	    || OpcodeIs( 'I', 'L' ) ) {
		Serial.write( "? " );
		varname = getDestVarname( &line, &next );
		valueA = Serial.read();
		storeVariable( varname, valueA, next );

		if( op1 == 'L' ) {
			while( valueA != '\n' ) {
				valueA = Serial.read();
			}
		}

		return kJRNextLine;
	}

	////////////////////////////////////////
	// Conversion Functions

	// AS - ASC() - convert ascii value to integer (atoi)
	if( OpcodeIs( 'A', 'S' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		if( valueA >= '0' || valueA <= '9' ) {
			valueA = valueA - '0';
		}
		else if( valueA >= 'a' && valueA <= 'z' ) {
			valueA = 10 + (valueA - 'a');
		}
		else if( valueA >= 'A' && valueA <= 'Z' ) {
			valueA = 10 + (valueA - 'A');
		} else {
			valueA = 0;
		}
		storeVariable( varname, valueA, next );
		return next;
	}



	////////////////////////////////////////
	// Misc?

	// WA - WAIT - waits for the specified milliseconds (1000 = 1s)
	if( OpcodeIs( 'W', 'A' )) {
		valueA = getParamValue( &line, &next );
		delay( valueA );
		return next;
	}

	// MS - millis() - get the number of millis since poweron
	if( OpcodeIs( 'M', 'S' )) {
		varname = getDestVarname( &line, &next );
		storeVariable( varname, millis(), next );
		return next;
	}

	// RN - LET D = RND( P ) - get a random number
	if( OpcodeIs( 'R', 'N' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		if( valueA < 1 ) valueA = 1;
		valueB = random( valueA + 1 );
		storeVariable( varname, valueB, next );
		return next;
	}

	// RA - RANDOMIZE( A ) - set the random seed
	if( OpcodeIs( 'R', 'A' )) {
		valueA = getParamValue( &line, &next );
		randomSeed( valueA );
		return next;
	}
	


	////////////////////////////////////////
	// VARIABLE ASSIGNMENT

	// LE - LET - set a variable
	if( OpcodeIs( 'L', 'E' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		storeVariable( varname, valueA, next );
		return next;
	}


	////////////////////////////////////////
	// PEEK and POKE

	// PE - PEEK - Look in a RAM address
	if( OpcodeIs( 'P', 'E' )) {
		// PE (To Var) (address)
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		valueB = programRam[ valueA % kRamSize ];
		storeVariable( varname, valueB, next );
		return next;
	}

	// PO - POKE - Store into a RAM address
	if( OpcodeIs( 'P', 'O' )) {
		// PO ( addr ) (value )
		valueA = getParamValue( &line, &next );
		do {
			valueB = getParamValue( &line, &next );
			if( next == kJRNextLine ) {
				programRam[ valueA % kRamSize ] = valueB;
				SKIP_WHITESPACE( line );
			}
			valueA++;
		} while( (next == kJRNextLine)
			&& (*line != '\n')
			&& (*line != '\0') );
		return next;
	}

	// EE - PEEK - Look in an EEPROM address
	if( OpcodeIs( 'E', 'E' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		valueB = EEPROM.read( valueA % kEESize );
		storeVariable( varname, valueB, next );
		return next;
	}

	// EO - POKE - Store int an EEPROM address
	if( OpcodeIs( 'E', 'O' )) {
		valueA = getParamValue( &line, &next );
	 	do {
			valueB = getParamValue( &line, &next );
			if( next == kJRNextLine ) {
				EEPROM.write( valueA % kEESize, valueB & 0x0ff );
				SKIP_WHITESPACE( line );
			}
			valueA++;
		} while( (next == kJRNextLine)
			&& (*line != '\n')
			&& (*line != '\0') );
		return next;
	}


	////////////////////////////////////////
	// MATH

	// M+ - LET A = B + C - addition
	if( OpcodeIs( 'M', '+' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		storeVariable( varname, valueA + valueB, next );
		return next;
	}

	// M- - LET A = B - C - subtraction
	if( OpcodeIs( 'M', '-' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		storeVariable( varname, valueA - valueB, next );
		return next;
	}

	// M/ - LET A = B / C - division
	if( OpcodeIs( 'M', '/' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		if( valueB == 0 ) {
			next = kJRDBZError;
		}
		storeVariable( varname, valueA / valueB, next );
		return next;
	}

	// M* - LET A = B * C - multiplication
	if( OpcodeIs( 'M', '*' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		storeVariable( varname, valueA * valueB, next );
		return next;
	}

	// MI - LET A = A+1 - increment variable
	if( OpcodeIs( 'M', 'I' )) {
		varname = getDestVarname( &line, &next );
		valueA = retrieveVariable( varname );
		storeVariable( varname, valueA+1, next );
		return next;
	}

	// MD - LET A = A-1 - decrement variable
	if( OpcodeIs( 'M', 'D' )) {
		varname = getDestVarname( &line, &next );
		valueA = retrieveVariable( varname );
		storeVariable( varname, valueA -1, next );
		return next;
	}


	////////////////////////////////////////	
	// BITWISE MATH

	// M< - LET A = B << C - shifts B left by C bits
	if( OpcodeIs( 'M', '<' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		storeVariable( varname, valueA << valueB, next );
		return next;
	}

	// M> - LET A = B >> C - shifts B right by C bits
	if( OpcodeIs( 'M', '>' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		storeVariable( varname, valueA >> valueB, next );
		return next;
	}

	// M& - LET A = B & C - bitwise "and"/"mask" of B with C
	if( OpcodeIs( 'M', '&' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		storeVariable( varname, valueA & valueB, next );
		return next;
	}

	// M| - LET A = B | C - bitwise "or"/"set" of B with C
	if( OpcodeIs( 'M', '|' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		storeVariable( varname, valueA | valueB, next );
		return next;
	}

	// M! - LET A = NOT B - bitwise inversion of B
	if( OpcodeIs( 'M', '!' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		storeVariable( varname, ~valueA, next );
		return next;
	}


	////////////////////////////////////////	
	// GOTOs

	// GO - GOTO - continue on the specified line
	if( OpcodeIs( 'G', 'O' )) {
		valueA = getParamValue( &line, &next );
		next = valueA;
		return next;
	}

	// G< - IF ( B < C ) GOTO A - Conditional greater-than GOTO
	if( OpcodeIs( 'G', '<' )) {
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		valueC = getParamValue( &line, &next );
		if( valueB < valueC ) next = valueA;
		return next;
	}

	// G> - IF ( B > C ) GOTO A - Conditional less-than GOTO
	if( OpcodeIs( 'G', '>' )) {
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		valueC = getParamValue( &line, &next );
		if( valueB > valueC ) next = valueA;
		return next;
	}

	if( OpcodeIs( 'G', '=' )) {
	// G= - IF ( B = C ) GOTO A - Conditional equality GOTO
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		valueC = getParamValue( &line, &next );
		if( valueB == valueC ) next = valueA;
		return next;
	}

	// CA - GOSUB - CALL subroutine A
	if( OpcodeIs( 'C', 'A' )) {
		valueA = getParamValue( &line, &next );
		doCall( valueA, bufc, &next );

		return next;
	}

	// C< - IF ( B < C ) GOSUB A - conditional less-than GOSUB
	if( OpcodeIs( 'C', '<' )) {
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		valueC = getParamValue( &line, &next );
		if( valueB < valueC ) {
			doCall( valueA, bufc, &next );
		}
		return next;
	}

	// C> - IF ( B > C ) GOSUB A - conditional greater-than GOSUB
	if( OpcodeIs( 'C', '>' )) {
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		valueC = getParamValue( &line, &next );
		if( valueB > valueC ) {
			doCall( valueA, bufc, &next );
		}
		return next;
	}

	// C= - IF ( B = C ) GOSUB A - conditional equality GOSUB
	if( OpcodeIs( 'C', '=' )) {
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		valueC = getParamValue( &line, &next );
		if( valueB == valueC ) {
			doCall( valueA, bufc, &next );
		}
		return next;
	}

	// CR - CALL-RETURN - return from a subroutine
	if( OpcodeIs( 'C', 'R' )) {
		if( gosubLevel <= 0 ) {
			return kJRGosubStack;
		} 
		gosubLevel--;
		*bufc = gosubStack[gosubLevel];
		gosubStack[gosubLevel] = NULL;
		next = kJRNextLine; // force this.

		return next;
	}


	////////////////////////////////////////
	// IF - conditional with GOto or CAll

	// IF P < Q GO D
	// IF P > Q GO D
	// IF P = Q GO D
	// IF P < Q CA D
	// IF P > Q CA D
	// IF P = Q CA D

	if( OpcodeIs( 'I', 'F' )) {
		valueA = getParamValue( &line, &next );
		SKIP_WHITESPACE( line );
		char cond = *line; /* should be < = > */
		if(   cond != '<'
		   && cond != '=' 
		   && cond != '>' ) next = kJRSyntaxError;
		line++;
		valueB = getParamValue( &line, &next );

		char opDO = getGoOrCa( &line, &next );

		valueC = getParamValue( &line, &next );

		// bail out should there be a mess
		if( next != kJRNextLine ) return next;

		// there's no good way to do this...
		if( opDO == kGoCa_GO ) {
			if( cond == '<' && (valueA < valueB )) next = valueC;
			if( cond == '>' && (valueA > valueB )) next = valueC;
			if( cond == '=' && (valueA == valueB )) next = valueC;
		} else if( opDO == kGoCa_CA ) {
			if( cond == '<' && (valueA < valueB ))
				doCall( valueC, bufc, &next );
			if( cond == '>' && (valueA > valueB ))
				doCall( valueC, bufc, &next );
			if( cond == '=' && (valueA == valueB ))
				doCall( valueC, bufc, &next );
		}
		return next;
	}

	if( OpcodeIs( 'O', 'N' )) {
		valueA = getParamValue( &line, &next );
		char opDO = getGoOrCa( &line, &next );

		for( int i = 0 ; i<=valueA && (next == kJRNextLine); i++ )
		{
			valueB = getParamValue( &line, &next );
			if( i == valueA )
			{
				if( opDO == kGoCa_GO ) return valueB;
				if( opDO == kGoCa_CA ) {
					doCall( valueB, bufc, &next );
					return next;
				}
			}
		}


		return next;
	}

	////////////////////////////////////////
	// Digital IO  analog/digital write/read

	// AW - analogWrite - Write to pin A the value B
	if( OpcodeIs( 'A', 'W' )) {
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		pinMode( valueA, OUTPUT );
		analogWrite( valueA, valueB );
		return next;
	}

	// DW - digitalWrite - Write to pin A the value B
	if( OpcodeIs( 'D', 'W' )) {
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		pinMode( valueA, OUTPUT );
		digitalWrite( valueA, (valueB==0)?LOW:HIGH );
		return next;
	}

	// AR - analogRead - Read into variable A, from pin B
	if( OpcodeIs( 'A', 'R' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		pinMode( valueA, INPUT );
		storeVariable( varname, analogRead( valueA ), next );
		return next;
	}

	// DR - digitalRead - Read into variable A, from pin B
	if( OpcodeIs( 'D', 'R' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		pinMode( valueA, INPUT );
		storeVariable( varname, digitalRead( valueA ), next );
		return next;
	}

	
	// little bit of a hack, but it'll save a few bytes.
	buf = line;
	buf[0] = op0;
	buf[1] = op1;
	buf[2] = '\0';
	Serialprint( "?OP: " );
	Serial.println( buf );
	return kJRStop;
}

void cmd_run( void )
{
	int cline = 0;
	int next = kJRFirstLine;
	char *bufc = programRam;

	gosubLevel = 0;
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
				Serialprintln( ": ?LINE" );
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
			Serialprint( "Gosub: " );
			Serial.print( (long) gosubLevel, DEC );
			Serialprintln( " items." );

			Serialprint( "Line: " );
			char * tc = bufc;
			while( (*tc) != '\0' && (*tc) != '\n' ) {
				Serial.write( (const uint8_t*)tc, 1 );
				tc++;
			}
			SerialprintNewline();


			// get user input (return)
			while( !Serial.available() );
			(void) Serial.read();
		}

		// do the thing!
		cline = myAtoi( bufc );

		next = evaluate_line( ln, &bufc );

#ifndef DESKTOP
                while( Serial.available() > 0 ) {
                  int ch = Serial.read();
                  if( ch == 3 || ch == 4 || ch == 90 ) {
                    // ctrl-c or ctrl-d or 'Z'
                      next = kJRInterrupt;
                  }
                }
#endif
                
                if( next == kJRInterrupt ) {
                        Serialprintln( "STOP." );
                        next = kJRStop;
                }

		if( next == kJRGosubStack ) {
			Serialprintln( "?GOSUB" );
			next = kJRStop;
		}
		if( next == kJRDBZError ) {
			Serialprintln( "?DBZ" );
			next = kJRStop;
		}
		if( next == kJRSyntaxError ) {
			Serialprintln( "?SYNTAX" );
			next = kJRStop;
		}
	}

	Serialprint( "\nSTOP @ " );
	Serial.println( (long)cline, DEC );
}

////////////////////////////////////////////////////////////////////////////////

void do_autoload( void )
{
	// peek at the first byte of the EEProm
	int ch = EEPROM.read( 0 );

	if( ch == 0x0ff ) {
		// unformatted. don't do anything.
		return;
	}

	if( ch == 0x00 ) {
		// formatted, empty. no reason to print out anything.
		return;
	}
	cmd_eload();
}


void do_startupOptions( void )
{
	int ch;

	// check for sentinel
	ch = EEPROM.read( E2END-2 );
	if( ch != 'B' ) return;
	ch = EEPROM.read( E2END-1 );
	if( ch != 'L' ) return;

	ch = EEPROM.read( E2END );

        //////////////////////////
        // Autoload
	// ok. let's check for the autorun flag
	if( ch & kFlagAutorun )
		cmd_run();
}



void setup()
{
	// set up serial port
	Serial.begin( kSerialBaud );
	while( !Serial ) {
		; // wait for Leonardo to catch up
	}

	Serialprintln( "BLuB " kBLuBVersion );
	cmd_new();
	init_vars();
	do_autoload();
	SerialprintNewline();
	cmd_mem();
	
	do_startupOptions();
} 




void cmd_removeLine( int lineNo, bool verbose )
{
	char *bufc = programRam;
	char *bufn = 0;

	while( *bufc ) {
		// if we're looking at '\0', error line not found, return
	    	if( *bufc == '\0' ) break;

		// check the current pointer for the matching line
		int cline = myAtoi( bufc );

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
		Serialprint( "Error: Line not found." );
}

void cmd_insertLine( char * theLine )
{
	// for now, we'll just shove it into the end of program ram
	// char programRam[ kRamSize ];

	long pgmSz = strlen( programRam );
	long newSz = strlen( theLine );

	if( (pgmSz + newSz) >= kRamSize )
	{
		Serialprintln( "Out of memory!" );
		return;
	}

	// find the appropriate location to insert it

	int lineNo = myAtoi( theLine );

	char *bufc = programRam;
	char *bufn = 0;

	while( *bufc ) {
		// if we're looking at '\0', error line not found, return
	    	if( *bufc == '\0' ) break;

		// check the current pointer for the matching line
		int cline = myAtoi( bufc );

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

int strMatches( char * haystack, const char * needle )
{
	if( !haystack || !needle ) return 0;

	while( *haystack != '\0' && *needle != '\0' )
	{
		if( *haystack != *needle ) return 0;

		haystack++;
		needle++;
	}

	if( *needle == '\0' ) return 1;
	return 0;
}

void loop()
{
	// print out something
	Serial.print( ":) " );
	
	// get a line of input
#ifdef kLocalEcho
	getSerialLine( linebuf, kLineLen, true );
#else
	getSerialLine( linebuf, kLineLen, false );
#endif

	bptr = linebuf;
	SKIP_WHITESPACE( bptr );

	// FUTURE (maybe)
	//	- Instead of using "strcmp", switch to just checking the 
	//	first word on the line, this will let us do command line
	//	parameters, rather than just checking only the first word
	//	on the line. 
	//	- store the command strings for comparison to PROGMEM

	// process it
	if( !strcmp( bptr, "mem" )) { cmd_mem(); }

	else if( !strcmp( bptr, "new" )) { cmd_new(); }
	else if( !strcmp( bptr, "list" )) { cmd_list(); }
	else if( !strcmp( bptr, "vars" )) { cmd_vars(); }
	else if( !strcmp( bptr, "clear" )) { init_vars(); }
	else if( !strcmp( bptr, "help" )) { cmd_help(); }

#ifdef DESKTOP
	else if( !strcmp( bptr, "exit" )) { exit( 0 ); }
#endif

	else if( !strcmp( bptr, "enew" )) { cmd_enew(); }
	else if( !strcmp( bptr, "elist" )) { cmd_elist(); }
	else if( !strcmp( bptr, "eload" )) { cmd_eload(); }
	else if( !strcmp( bptr, "esave" )) { cmd_esave(); }

#ifdef kIncludeMassStorage
	else if( !strcmp( bptr, "msnew" )) { cmd_msnew(); }
	else if( !strcmp( bptr, "msbanks" )) { cmd_msbanks(); }
	else if( strMatches( bptr, "msload" )) { 
		bptr+= 6;
		SKIP_WHITESPACE( bptr );
		cmd_msload( bptr );
	}
	else if( strMatches( bptr, "mssave" )) {
		bptr+= 6;
		SKIP_WHITESPACE( bptr );
		cmd_mssave( bptr );
	}
#endif

	else if( !strcmp( bptr, "tron" )) { 
		Serial.println( "Trace on." );
		trace = true;
	}
	else if( !strcmp( bptr, "troff" )) {
		Serial.println( "Trace off." );
		trace = false;
	}
	else if( !strcmp( bptr, "run" )) { cmd_run(); }

	else if( bptr[0] >= '0' && bptr[0] <= '9' ) {
		// it's starting with a number

		// get the line number
		int v = myAtoi( bptr );
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
			// handling for pre-whitespace
			bptr = linebuf;
			SKIP_WHITESPACE( bptr );
			cmd_insertLine( bptr );
		}

	}
	else if( linebuf[0] != '\0' ){
		Serialprintln( "Huh?" );
	}

cleanup:
	SerialprintNewline();
}

#ifdef NEVER
/* this is the playground area for the SD card functions.
   SD Card IO will be accomplished as bare segments of the drive, 
   much like EEPROM.

    Block 0: (reserved, unused for now)
    Block 1: (first content block)
    Block 2: (second content block)
		...

    Content Blocks:
	byte 0..79    Null-backfilled filename 
	byte 80..511  Content

    snew - format SD card (quick format, skip to block 1, write a 0)
    scatalog - display list of filenames
	block = 1
	{
	    if byte[0] != '\0':
		print block number ;
		read block,  dump to serial until '\0'
		    consume rest of block
		OR
		    consume to 80 bytes
		    count up number of bytes until NULL
		    display that number in bytes
	    blocK++
	}

    ssave <block> <filename>

	if( block <1  ||  block > 100 ) ERROR
	seek to block number
	write block
	    write out filename
	    pad to 80
	    write out string
	    pad to blocksize

    sload <block>
	seek to block number
	read block
	    skip 80 bytes
	    read in string to ram
	    stop when end of string or ram buffer
	    skip to end of buffer
*/

int sdcard_open( void )
{
	/* put card in SPI mode */
	/* query card size */
	Serialprintln( "No SD." );
	return -1;
}

void sdcard_seekblock( int block )
{
}

void sdcard_catalog( void )
{
	if( !scard_open() ) return;
	/* seek block 1 */
}

void sdcard_write( char * buffer )
{
	/* write out the buffer */
	/* pad output to 512 bytes */
	if( !scard_open() ) return;

}


#endif
