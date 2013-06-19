// ARDSIM.H
//
//	Include all the headers for the Arduino simulation stuff


// EEPROM simuator
#include "eesim.h"

// Data line IO
#include "iosim.h"

// Misc stuff - Delay/timer, random, etc.
#include "miscsim.h"

// Serial IO
#include "sersim.h"



#ifndef FLASHEND
#define FLASHEND (0x0fff)
#endif

#ifndef RAMEND
#define RAMEND (0x0fff)
#endif

#ifndef boolean
typedef bool boolean;
#endif
