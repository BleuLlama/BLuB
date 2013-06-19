
#ifndef HIGH
#define HIGH 1
#endif

#ifndef LOW
#define LOW 0
#endif

#define kAD_Digital	(0)
#define kAD_Analog	(1)

#define kIO_unknown	(0)
#define kIO_Input	(1)
#define kIO_Output	(2)

#ifndef OUTPUT
#define OUTPUT	(kIO_Output)
#endif
#ifndef INPUT
#define INPUT	(kIO_Input)
#endif

#ifndef INPUT_PULLUP
#define INPUT_PULLUP (kIO_Input)
#endif

#ifndef D0
#define D0	0
#define D1	1
#define D2	2
#define D3	3
#define D4	4
#define D5	5
#define D6	6
#define D7	7
#define D8	8
#define D9	9
#define D10	10
#define D11	11
#define D12	12
#define D13	13

// Digital overlay of analog inputs
#define D14	14
#define D15	15
#define D16	16
#define D17	17
#define D18	18
#define D19	19

// Analog inputs
#define A0	14
#define A1	15
#define A2	16
#define A3	17
#define A4	18
#define A5	19
#endif

#define kNPins	(19)

struct aPin {
	int id;
	int ad_mode;
	int io_mode;
	int value;
};

void pinMode( int pin, int mode );

int digitalRead( int pin );
int analogRead( int pin );

void digitalWrite( int pin, int value );
void analogWrite( int pin, int value );

void io_dump( void );
