

#include <iostream>
#include "iosim.h"

/*
struct aPin {
	int id;
	int ad_mode;
	int io_mode;
	int value;
}

*/

static aPin pins[ kNPins ];
static bool initialized = false;

void pinInit( void )
{
	if( initialized ) return;
	for( int i =0 ; i<kNPins ; i++ ) {
		pins[i].id = i;
		pins[i].ad_mode = kAD_Digital;
		pins[i].io_mode = kIO_unknown;
		pins[i].value = 0;
	}
}

void pinMode( int pin, int mode )
{
	pinInit();
	if( pin < 0 || pin > kNPins ) return;

	pins[ pin ].io_mode = (mode == INPUT)? kIO_Input : kIO_Output;
}

int digitalRead( int pin )
{
	pinInit();
	if( pin < 0 || pin > kNPins ) return LOW;
	pins[ pin ].ad_mode = kAD_Digital;
	return pins[ pin ].value;
}

int analogRead( int pin )
{
	pinInit();
	if( pin < 0 || pin > kNPins ) return 0;
	pins[ pin ].ad_mode = kAD_Analog;
	return pins[ pin ].value;
}

void digitalWrite( int pin, int value )
{
	pinInit();
	if( pin < 0 || pin > kNPins ) return;
	pins[ pin ].ad_mode = kAD_Digital;
	pins[ pin ].value = value;
}


void analogWrite( int pin, int value )
{
	pinInit();
	if( pin < 0 || pin > kNPins ) return;
	pins[ pin ].ad_mode = kAD_Analog;
	pins[ pin ].value = value;
}

void io_dump( void )
{
	pinInit();

	std::cout << "========== IO:" << std::endl;
	for( int i=0 ; i<kNPins ; i++ )
	{
		printf( " %2d: ", i );

		switch( pins[i].io_mode ){
		case( kIO_Input ):
			std::cout << "input " << pins[i].value << std::endl;
			break;

		case( kIO_Output ):
			std::cout << "output " << pins[i].value << std::endl;
			break;

		case( kIO_unknown ):
		default:
			std::cout << "unset" << std::endl;
		}
			
	}
	std::cout << std::endl;
}
