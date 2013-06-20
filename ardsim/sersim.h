// sersim
//
// 	simulate the Serial library in Arduinosville

#include <iostream>

class sersim {
private:

public:
	sersim( void );
	~sersim( void );

	bool operator!(void); 	// for leonardo sim

public:
	void begin( long speed, long config = 0 );
	void end( void );

	int available( void ); // returns number of bytes to read
	int peek( void );

#ifndef BIN 
    #define BIN	(1)
    #define OCT	(8)
    #define DEC	(10)
    #define HEX	(16)
#endif


	void print( long v, int fmt=DEC ); // BIN, OCT, DEC, HEX
	void print( double v, int prec=2); // bits of precision after decimal point
	void print( std::string v );

	void println( long v, int fmt=DEC ); // BIN, OCT, DEC, HEX
	void println( double v, int prec=2 ); // bits of precision after decimal point
	void println( std::string v );

	void write( long v );
	void write( std::string v );
	void write( char * v, long len );
	void write( const uint8_t * v, long len ) {
		this->write( (char *)v, len );
	}

	void flush( void );
	
	int read( void );
};

extern sersim Serial;
