
#include <iostream>

// hooks into the .ino
extern void setup( void );
extern void loop( void );


int main( int argc, char ** argv )
{
	std::cout << "BLuB Starting up..." << std::endl;
	setup();

	while( 1 ) {
		loop();
	}
}
