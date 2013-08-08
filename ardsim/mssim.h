// mssim
//	SD/Mass Storage simulator
//

////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#define kMSSize		(1024 * 1024 * 8) 	/* 8 meg */
#define kMSBlockSize 	(512)			/* SDCard Standard */


class ms_simulator {
private:
	std::string fn;
	long sz;
	unsigned char * memory;

public:
	ms_simulator( std::string filename, long size );
	~ms_simulator( void );

public:
	void Open( void );
	void Close( void );
	void Format( void );

public:
	void Load( void );
	void Save( void );

	void erase( long block );

	unsigned char read( long block, long addr );
	void write( long block, long addr, unsigned char value );
};

extern ms_simulator MASSSTORAGE;
