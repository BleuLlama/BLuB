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
	sz++;
	this->memory = (unsigned char *) malloc( sizeof( unsigned char) * sz );
	this->Load();
}

eeprom_simulator::~eeprom_simulator( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void eeprom_simulator::Load( void )
{
	if( !this->memory ) return;

	FILE * fp = fopen( this->fn.c_str(), "r" );
	if( !fp ) return;

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


eeprom_simulator EEPROM( "eeprom.dat", (EE2END+1) );
