
#include <iostream>
#include <unistd.h> 	// for usleep
#include <sys/time.h>	// for timeval

////////////////////////////////////////////////////////////////////////////////
// time functions

void delay( long v )
{
	usleep( v * 1000 );
}

long millis( void )
{
        static timeval startTime;

        timeval endTime;
        long seconds, useconds;
        double duration;

        if( startTime.tv_sec == 0 && startTime.tv_usec == 0 ) {
                gettimeofday( &startTime, NULL );
        }

        gettimeofday( &endTime, NULL );
        seconds = endTime.tv_sec - startTime.tv_sec;
        useconds = endTime.tv_usec - startTime.tv_usec;

        duration = seconds + useconds/1000000.0;

        return( (long) (duration * 1000) );
}
