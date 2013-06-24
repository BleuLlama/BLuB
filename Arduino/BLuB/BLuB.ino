// BLuB
//	BleuLlama's microBasic
//
//	A basic-inspired, asm-ish programming language thing
//

////////////////////////////////////////////////////////////////////////////////
// Version history

#define kBLuBVersion	"v0.05  2013-June-24  yorgle@gmail.com"

// v0.05  2013-June-24  rearranged opcode names, added IC/L
//			better documentation
//			added pre-text whitespace elimination
//			WA, RN, RA, AS
//			PRint changed from PP
//			CAll added (GOSUB)
//			IF statement added
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

#define kNGosubs (15)
char gosubLevel = 0;
char * gosubStack[ kNGosubs ];

#define kJRGosubStack	(-6)
#define kJRDBZError	(-5)
#define kJRSyntaxError	(-4)
#define kJRStop		(-3)
#define kJRNextLine	(-2)
#define kJRFirstLine	(-1)

////////////////////////////////////////////////////////////////////////////////

#define kBufLen (16)
char buffer[kBufLen];

#define PRINT_LINE( L ) \
	{ \
		for( char * tl = (L) ; *tl != '\n' && *tl != '\0' ; tl++ ) \
			Serial.write( tl, 1 ); \
		Serial.println( "" ); \
	}
		
#define SKIP_NUMBER( A ) \
	while(     (*A) >= '0' \
		&& (*A) <= '9' \
		&& (*A) != '\0' ) (A)++;

#define SKIP_WHITESPACE( A ) \
	while( (   (*A) == ' ' \
		|| (*A) == '\t' \
		|| (*A) == ',' \
	        ) && (*A) != '\0' ) (A)++; /* parens here are IMPORTANT */

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
#ifdef DESKTOP
	Serial.println( "    files   load    save" );
#endif
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
    if( echoback ) Serial.write( buf[chp] );
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
	Serial.print( "Clearing EEPROM " );
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
		Serial.write( (const uint8_t *) &ch, 1 );
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


////////////////////////////////////////////////////////////////////////////////
#ifdef DESKTOP_NEVER
// a few commands for desktop use.

#include <dirent.h>

#define kProgramFolder	"programs"

void cmd_files( void )
{
	DIR * drp = NULL;
	struct dirent * dent = NULL;
	int count = 0;

	drp = opendir( kProgramFolder );
	if( !drp ) {
		Serial.print( kProgramFolder );
		Serial.println( ": Unable to read folder." );
		return;
	}

	Serial.println( "" );
	
	while( ((dent = readdir( drp )) != NULL ) )
	{
		if( dent->d_type != DT_DIR ) {
			Serial.print( "    " );
			Serial.println( dent->d_name );
			count++;
		}
	}
	closedir( drp );

	if( count != 0 ) Serial.println( "" );

	Serial.print( (long) count, DEC );
	if( count == 1 ) 
		Serial.println( " files found." );
	else
		Serial.println( " file found." );
}

void cmd_load( void )
{
	char fname[ 64 ]
}

void cmd_save( void )
{
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
}

void cmd_vars( void )
{
	char buf[20];
	Serial.println( "Variables:" );
	for( int i=0 ; i<(kNVariables/2) ; i++ )
	{
		snprintf( buf, 20, "  %c: %-9d", i+'a', variables[i] );
		Serial.print( buf );

		snprintf( buf, 20, "    %c: %-9d", i+13+'a', variables[i+13] );
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
	// PL - PRINT - print out a variable, parameter or string, with newline
	if(    OpcodeIs( 'P', 'R' )
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
			Serial.print( (long) valueA, DEC );
		}
		if( op1 == 'L' ) {
			Serial.println( "" );
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

	// RN - LET D = RND( P ) - get a random number
	if( OpcodeIs( 'R', 'N' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		valueB = random( valueA );
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
		return kJRNextLine;
	}

	// PO - POKE - Store into a RAM address
	if( OpcodeIs( 'P', 'O' )) {
		return kJRNextLine;
	}

	// EE - PEEK - Look in an EEPROM address
	if( OpcodeIs( 'E', 'E' )) {
		return kJRNextLine;
	}

	// EO - POKE - Store int an EEPROM address
	if( OpcodeIs( 'E', 'O' )) {
		return kJRNextLine;
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
		SKIP_WHITESPACE( line );
		char opY = *line; line++;
		char opZ = *line; line++;
		char opDO = 0;
		if( opY == 'G' && opZ == 'O' ) opDO = 1;
		if( opY == 'C' && opZ == 'A' ) opDO = 2;
		if( opDO == 0 ) next = kJRSyntaxError;
		valueC = getParamValue( &line, &next );

		// bail out should there be a mess
		if( next != kJRNextLine ) return next;

		// there's no good way to do this...
		if( opDO == 1 ) {
			if( cond == '<' && (valueA < valueB )) next = valueC;
			if( cond == '>' && (valueA > valueB )) next = valueC;
			if( cond == '=' && (valueA == valueB )) next = valueC;
		} else if( opDO == 2 ) {
			if( cond == '<' && (valueA < valueB ))
				doCall( valueC, bufc, &next );
			if( cond == '>' && (valueA > valueB ))
				doCall( valueC, bufc, &next );
			if( cond == '=' && (valueA == valueB ))
				doCall( valueC, bufc, &next );
		}
		return next;
	}

	////////////////////////////////////////
	// Digital IO  analog/digital write/read

	// AW - analogWrite - Write to pin A the value B
	if( OpcodeIs( 'A', 'W' )) {
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		analogWrite( valueA, valueB );
		return next;
	}

	// DW - digitalWrite - Write to pin A the value B
	if( OpcodeIs( 'D', 'W' )) {
		valueA = getParamValue( &line, &next );
		valueB = getParamValue( &line, &next );
		digitalWrite( valueA, (valueB==0)?LOW:HIGH );
		return next;
	}

	// AR - analogRead - Read into variable A, from pin B
	if( OpcodeIs( 'A', 'R' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		storeVariable( varname, analogRead( valueA ), next );
		return next;
	}

	// DR - digitalRead - Read into variable A, from pin B
	if( OpcodeIs( 'D', 'R' )) {
		varname = getDestVarname( &line, &next );
		valueA = getParamValue( &line, &next );
		storeVariable( varname, digitalRead( valueA ), next );
		return next;
	}

	Serial.println( "Unknown opcode." );
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
			Serial.print( "Gosub Stack: " );
			Serial.print( (long) gosubLevel, DEC );
			Serial.println( " items." );

			Serial.print( "Line: " );
			char * tc = bufc;
			while( (*tc) != '\0' && (*tc) != '\n' ) {
				Serial.write( (const uint8_t*)tc, 1 );
				tc++;
			}
			Serial.println( "" );


			// get user input (return)
			while( !Serial.available() );
			(void) Serial.read();
		}

		// do the thing!
		cline = myAtoi( bufc );

		next = evaluate_line( ln, &bufc );

		if( next == kJRGosubStack ) {
			Serial.println( "Gosub stack error." );
			next = kJRStop;
		}
		if( next == kJRDBZError ) {
			Serial.println( "Divide By Zero Error." );
			next = kJRStop;
		}
		if( next == kJRSyntaxError ) {
			Serial.println( "Syntax Error." );
			next = kJRStop;
		}
	}

	Serial.println( "" );
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

void loop()
{
	// print out something
	Serial.print( ":) " );
	
	// get a line of input
	getSerialLine( linebuf, kLineLen, false );

	bptr = linebuf;
	SKIP_WHITESPACE( bptr );

	// process it
	if( !strcmp( bptr, "mem" )) { cmd_mem(); }

	else if( !strcmp( bptr, "new" )) { cmd_new(); }
	else if( !strcmp( bptr, "list" )) { cmd_list(); }
	else if( !strcmp( bptr, "vars" )) { cmd_vars(); }
	else if( !strcmp( bptr, "help" )) { cmd_help(); }

#ifdef DESKTOP_NEVER
	else if( !strcmp( bptr, "files" )) { cmd_files(); }
	else if( !strcmp( bptr, "load" )) { cmd_load(); }
	else if( !strcmp( bptr, "save" )) { cmd_save(); }
#endif

	else if( !strcmp( bptr, "enew" )) { cmd_enew(); }
	else if( !strcmp( bptr, "elist" )) { cmd_elist(); }
	else if( !strcmp( bptr, "eload" )) { cmd_eload(); }
	else if( !strcmp( bptr, "esave" )) { cmd_esave(); }

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
		Serial.println( "Huh?" );
	}

cleanup:
	Serial.println( "" );
}
