// sersim
//
// 	simulate the Serial library in Arduinosville


#include <iostream>
#include "sersim.h"

////////////////////////////////////////////////////////////////////////////////

sersim::sersim( void )
{
}
	
sersim::~sersim( void )
{
}


bool sersim::operator!( void )
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////


void sersim::begin( long speed, long config )
{
}


void sersim::end( void )
{
}

////////////////////////////////////////////////////////////////////////////////

int sersim::available( void )
{
	 // should return the number of bytes to read

	return 1;

/*
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	return (FD_ISSET(0, &fds));
*/
}

int sersim::peek( void )
{
	// Don't know how to implement this
	int ch = getc( stdin );
	if( ch != EOF )
	ungetc( ch, stdin );
	return ch;
}


int sersim::read( void )
{
	int ch = getc( stdin );
	return ch;
}


////////////////////////////////////////////////////////////////////////////////

#ifndef BIN 
#define BIN	(1)
#define OCT	(8)
#define DEC	(10)
#define HEX	(16)
#endif


void sersim::print( long v, int fmt ) // BIN, OCT, DEC, HEX
{
	char buf[ 128 ];
	int p=0;

	switch( fmt ) {
	case( BIN ):
		int bit;
		for( bit = 0x80 ; bit>0 ; bit>>=1 )
		{
			buf[p] = (v&bit)?'1':'0';
			p++;
		}
		buf[p] = '\0';
		break;

	case( OCT ):
		snprintf( buf, 128, "%lo", v );
		break;

	case( HEX ):
		snprintf( buf, 128, "%02lx", v );
		break;

	case( DEC ):
	default:
		snprintf( buf, 128, "%ld", v );
		break;
	}

	std::cout << buf << std::flush;
}

void sersim::print( double v, int prec ) // bits of precision after decimal point
{
	char buf[128];
	char fmt[128];
	snprintf( fmt, 128, "%%0.%df", prec );
	snprintf( buf, 128, fmt, v );
	std::cout << buf << std::flush;
}

void sersim::print( std::string v )
{
	std::cout << v << std::flush;
}

void sersim::println( long v, int fmt ) // BIN, OCT, DEC, HEX
{
	this->print( v, fmt );
	this->print( "\n" );
}

void sersim::println( double v, int prec ) // bits of precision after decimal point
{
	this->print( v, prec );
	this->print( "\n" );
}

void sersim::println( std::string v )
{
	this->print( v );
	this->print( "\n" );
}

void sersim::write( long v )
{
	unsigned char buf[ 3 ];
	buf[0] = (unsigned char) (v & 0x00ff);
	buf[1] = '\0';
	std::cout << buf << std::flush;
}

void sersim::write( std::string v )
{
	std::cout << v << std::flush;
}

void sersim::write( char * v, long len )
{
	if( !v ) return;

	for( int a=0 ; a<len ; a++ )
	{
		std::cout << v[a];
	}
	std::cout << std::flush;
}

void sersim::flush( void )
{
	std::cout << std::flush;
}

sersim Serial;
