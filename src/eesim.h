// eeprom
//	eeprom simulator
//

////////////////////////////////////////////////////////////////////////////////

#include <iostream>

// simulated size of eeprom
#define EE2END	(0x0fff)


class eeprom_simulator {
private:
	std::string fn;
	long sz;
	unsigned char * memory;

public:
	eeprom_simulator( std::string filename, long size );
	~eeprom_simulator( void );

public:
	void Load( void );
	void Save( void );

	unsigned char read( long addr );
	void write( long addr, unsigned char value );
};

extern eeprom_simulator EEPROM;
