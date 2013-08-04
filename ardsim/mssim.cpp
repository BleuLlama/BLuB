// mssim
//	SD/Mass Storage simulator
//

////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "stdio.h"
#include "mssim.h"


//////////////////////////
// mass storage simulator

////////////////////////////////////////////////////////////////////////////////
	
ms_simulator::ms_simulator( std::string filename, long size )
	: fn( filename )
	, sz( size )
{
	this->memory = (unsigned char *) malloc( sizeof( unsigned char) * sz );
}

ms_simulator::~ms_simulator( void )
{
	if( this->memory ) free( this->memory );
}

////////////////////////////////////////////////////////////////////////////////

void ms_simulator::Open( void )
{
	this->Load();
}

void ms_simulator::Close( void )
{
	this->Save();
}

void ms_simulator::Format( void )
{
	// force in a header..
	this->memory[0] = 'B';
	this->memory[1] = 'L';
	this->memory[2] = 'u';
	this->memory[3] = 'B';
	this->memory[4] = '0';
	this->memory[5] = '1';
	this->memory[6] = '\0';

	// erase the first 100 blocks
	for( int i = 1 ; i <= 101 ; i++ ) {
		this->memory[ i * kMSBlockSize ] = '\0';
	}
}

void ms_simulator::Load( void )
{
	if( !this->memory ) return;

	FILE * fp = fopen( this->fn.c_str(), "r" );
	if( !fp ) {
		// no EEPROM found, create one with all FFs
		// (this is the initial state of EEPROM on device)
		memset( this->memory, 0xff, this->sz );
		this->Save();
		return;
	}

	// ok. it opened.  read it in.
	fread( this->memory, this->sz, 1, fp );
	fclose( fp );
}

void ms_simulator::Save( void )
{
	if( !this->memory ) return;

	FILE * fp = fopen( this->fn.c_str(), "w" );
	if( !fp ) return;
	fwrite(  this->memory, this->sz, 1, fp );
	fclose( fp );
}

unsigned char ms_simulator::read( long block, long addr )
{
	addr += (block * kMSBlockSize );
	return this->memory[ addr % this->sz ];
}

void ms_simulator::write( long block, long addr, unsigned char value )
{
	addr += (block * kMSBlockSize );
	this->memory[ addr % this->sz ] = value;
}


ms_simulator MASSSTORAGE( "mass.dat", kMSSize );
