// sersim
//
// 	simulate the Serial library in Arduinosville


#include <iostream>
#include "sersim.h"

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



void sersim::begin( long speed, long config )
{
	std::cout << "Begin Serial" << std::endl;
}


void sersim::end( void )
{
	std::cout << "Close Serial" << std::endl;
}

int sersim::available( void )
{
	 // returns number of bytes to read
	return 0;
}

int sersim::peek( void )
{
	return 0;
}


#ifndef BIN 
#define BIN	(1)
#define OCT	(8)
#define DEC	(10)
#define HEX	(16)
#endif


void sersim::print( long v, int fmt ) // BIN, OCT, DEC, HEX
{
}

void sersim::print( double v, int prec ) // bits of precision after decimal point
{
}

void sersim::print( std::string v )
{
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
}

void sersim::write( std::string v )
{
}

void sersim::write( char * v, long len )
{
}

void sersim::flush( void )
{
	std::cout << std::flush;
}

int sersim::read( void )
{
	return 0;
}

sersim Serial;
