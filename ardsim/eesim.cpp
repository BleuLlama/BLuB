// eeprom
//	eeprom simulator
//

////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "stdio.h"
#include "eesim.h"


////////////////////
// eeprom simulator

////////////////////////////////////////////////////////////////////////////////
	
eeprom_simulator::eeprom_simulator( std::string filename, long size )
	: fn( filename )
	, sz( size )
{
	this->memory = (unsigned char *) malloc( sizeof( unsigned char) * sz );
	this->Load();
}

eeprom_simulator::~eeprom_simulator( void )
{
	if( this->memory ) free( this->memory );
}

////////////////////////////////////////////////////////////////////////////////

void eeprom_simulator::Load( void )
{
	if( !this->memory ) return;

	FILE * fp = fopen( this->fn.c_str(), "r" );
	if( !fp ) {
		// no EEPROM found, create one with all FFs
		// (this is the initial state of EEPROM on device)
		memset( this->memory, 0xff, this->sz );
		return;
	}

	// ok. it opened.  read it in.
	fread( this->memory, this->sz, 1, fp );
	fclose( fp );
}

void eeprom_simulator::Save( void )
{
	if( !this->memory ) return;

	FILE * fp = fopen( this->fn.c_str(), "w" );
	if( !fp ) return;
	fwrite(  this->memory, this->sz, 1, fp );
	fclose( fp );
}

unsigned char eeprom_simulator::read( long addr )
{
	return this->memory[ addr % this->sz ];
}

void eeprom_simulator::write( long addr, unsigned char value )
{
	this->memory[ addr % this->sz ] = value;
	this->Save();
}


eeprom_simulator EEPROM( "eeprom.dat", (E2END+1) );
