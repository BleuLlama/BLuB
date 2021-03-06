BLuB / BLu Basic
====

BLuB A small variant of a BASIC style language, simplifying the syntax and
borrowing some concepts from ASM. "BLuB" originally stood for "BleuLlama's
micro-BASIC", but now it can also be shortened down to "BLuB" although 
"BLu Basic" is alternately preferred.  (There are other things out there
claiming to be a "BASIC" but are much further from Dartmouth Basic than
BLuB is.)


# Overview

The focus of this is not to be accurate to a standard BASIC syntax,
but instead use BASIC and its ease of use to be inspiration for a
small programmers shell for use on very low footprint
microcontrollers.

The project provides two methods for building it:

1. Unixey desktop.  This target is for building it for runtime on a 
   desktop machine (Mac OSX, MinGW on Windows, Linux) for use of learning
   or for debugging without a microcontroller.  This is accomplished via
   use of the content in the "ardsim" subdirectory.  This uses C++ 
   overloading to provide exact replacements for the ARDUINO interface
   without changing any code in the .INO Arduino source file.

2. Arduino source.  This target is for use for building it for use on
   Arduino devices.  Just double-click the .INO file, and it will open 
   inside of the Arduino IDE. (You may instead need to right-click it to 
   open it with the Arduino IDE, or copy it to your Arduino sketch folder.)

This does not provide an interface for SD or other storage, and none
is planned.  This is to keep everything small and self-contained.  It 
uses RAM for the primary runtime space, and EEPROM for power-off storage.

It is expected that the serial interface will be used to input programs
to be stored into the EEPROM, as well as for offline storage via text
capture of program listings.

BLuB has been optimized to maximize RAM space, so that it is completely
usable on an ATmega 168 part.  If there was a choice that would trade-off
progmem usage versus RAM, I went with progmem.  This means that the 
RAM footprint is small at the expense, in some cases, of PROGMEM usage.

--

# Syntax and Command Opcodes

The user-interaction interface for this is serial, and the desktop
interface is a line based text input.

(Line Number) (Opcode) (Optional Operands)

- Line number is an immediate integer base-10 numerical digit.
- Opcode is two uppercase letters
- Operands are lowercase letters for variables
- Operands are integer base-10 digits for immediate numbers
- Operands of multiple digits can be separated with a comma or whitespace
- Operands of string type start with a double quote to another dquote 

Some notes:  Whitespace between portions of the line are optional. For 
reading opcodes, the first two non-whitespace characters are considered
to be the opcode. The uppercase letters can continue until the operands
start (lowercase variable letter names, or numerical digits) So, for example,
the following little programs are all synonymous:

	10 RE example program
	20 PR "Hello, World!"
	30 LE a 42
	40 MI a
	50 IF a < 100 GO 40

and:

	10 REM example program
	20 PRINT "Hello, World!"
	30 LET a 42
	40 MINCREMENT a
	50 IF a < 100 GO 40

and:

	20PR"Hello, World!"
	30LEa42
	40MIa
	50IFa<100GO40
        
And so on.  This means that you can be as verbose in your program listing
as you'd like, or as short to try to preserve EEPROM/RAM usage.


# Variables

There are 26 variables, indicated by lowercase letters.  Some
variables are preset with values at the beginning of runtime.  (true,
false, zero, half, etc)  These can be restored to their initial
values (all zero except for the ones indicated below) by running
the 'clear' command.

- t = 255
- f = 0
- z = 0
- h = 128
- e = (size of the EEProm)
- r = (size of the RAM buffer)

The remaining variables are initialized to '0' at start of runtime.


# Interface Commands

These commands are typed in at the BLuB prompt by the user

- mem - display amount of free space in RAM and EEPROM
- help - display a list of commands and version information
- new - clear program memory, initialize variables
- vars - display variables
- clear - clears variables, and resets to default values
- list - display the program loaded into memory
- run - run the program in memory
- tron - turn program runtime trace on
- troff - turn program runtime trace off
- enew - clear EEPROM
- elist - display the program stored in EEPROM
- eload - load the program from EEPROM to memory
- esave - save the program from memory to EEPROM


# Program Opcodes

        BLuB Opcode         Definition using BASIC syntax

    System
        RE                  REM - add a comment
        EN                  END - end runtime

    Text IO
        PR  (P)             PRINT ; - print a parameter or string
        PL  (P)             PRINT   - output a parameter or string (newline)
        PC  (D)             PRINT (D) - output ascii data as character

        IC  (D)             INCHR - LET D = (newly INPUTTED character)
        IL  (D)             same as IC, but then ignore to end of line.

        AS  (D)             convert the ascii '0'-'9' to values 0-9

    Variable Assignment
        LE  (D) (P)         LET D = P

    Peek and Poke
        PE  (D) (A)         LET D = PEEK( A ) - get from RAM
        PO  (A) (V) ...     POKE( A, V )      - store V in RAM
        EE  (D) (A)         LET D = PEEK( A ) - get from EEPROM
        EO  (A) (V) ...     POKE( A, V )      - store V in EEPROM
	-- note: PO and EO can poke multiple sequential bytes

    Math
        M+  (D) (P) (Q)     LET D = P + Q
        M-  (D) (P) (Q)     LET D = P - Q
        M/  (D) (P) (Q)     LET D = P / Q
        M*  (D) (P) (Q)     LET D = P * Q

        MI  (D)             LET D = D + 1
        MD  (D)             LET D = D - 1

    Bitwise operations
        M<  (D) (P)         LET D = D << P  (left shift)
        M>  (D) (P)         LET D = D >> P  (right shift)
        M&  (D) (P)         LET D = D & P   (bit mask)
        M|  (D) (P)         LET D = D | P   (bit set)
        M!  (D)             LET D = ~D      (invert bits)

    Conditionals
        IF (P) < (Q) GO (D)  IF P<Q THEN GOTO D
        IF (P) = (Q) GO (D)  IF P=Q THEN GOTO D
        IF (P) > (Q) GO (D)  IF P>Q THEN GOTO D
        IF (P) < (Q) CA (D)  IF P<Q THEN GOSUB D
        IF (P) = (Q) CA (D)  IF P=Q THEN GOSUB D
        IF (P) > (Q) CA (D)  IF P>Q THEN GOSUB D

        ON (P) GO (D) ... (D)  P selects a zero-indexed item from the D list
        ON (P) CA (D) ... (D)
        
    GOTO
        GO  (D)             GOTO D
        G<  (D) (P) (Q)     IF ( P > Q ) THEN GOTO D
        G>  (D) (P) (Q)     IF ( P < Q ) THEN GOTO D
        G=  (D) (P) (Q)     IF ( P = Q ) THEN GOTO D

    GOSUB/CALL
        CA  (D)             CALL D
        C<  (D) (P) (Q)     IF ( P > Q ) THEN CALL D
        C>  (D) (P) (Q)     IF ( P < Q ) THEN CALL D
        C=  (D) (P) (Q)     IF ( P = Q ) THEN CALL D
        CR                  RETURN


    Digital IO
        AW  (P) (V)         analog write the value V to pin P
        DW  (P) (V)         digital write the value V to pin P
        AR  (D) (P)         analog read the value of pin P to varable D
        DR  (D) (P)         digital read the value of pin P to varable D

    Misc 
        WA  (D)             WAIT D  (milliseconds)
        MS  (D)             LET D = millis()
        AS  (D) (P)         LET D = ASC( P )
        RN  (D) (P)         LET D = RND( P )   (0..P)
        RA  (P)             RANDOMIZE( P )     (set random seed)

# Example Programs

## Fibonacci Sequence 

### BASIC version (for comparisons)

            10 REM fibonacci sequence
            20 LET a = 1
            30 LET b = 1
            40 FOR g = 0 TO 20
            50 LET c = a + b
            60 PRINT a ; " " ;
            70 LET a = b
            80 LET b = c
            90 NEXT g
            100 END


### BLuB version
Note that the BLuB verson uses a conditional GOTO instead of the FOR-NEXT
construct, as there is no FOR-NEXT in BLuB.  I tried to keep the line 
numbers lined up so that you can easily compare the two code blocks.

            10 RE Fibonacci Sequence
            20LEa1
            30 LE b 2
            40 LE g 0
            50 M+cab
            60 PRa
            61 PR" "
            70 LE ab
            80 LE bc
            90 MI g
            91 G< 50 g 20
            100 EN


## Display User Input

Prompt the user for input five times.  'a' is used as the counter.
The inputted character is stored in 'n' and then ascii-value (atoi) 
converted into 'm'.

	    10 RE Display user input five times
	    20 LEa0
	    30 PR "Loop number " 
 	    40 LEba
	    50 MIb
	    60 PRb
	    70 PL " of 5:"
	    80 PR "Type in a number "
	    90 ILn
	    100 PR "You typed ascii code "
	    110 PLn
	    120 ASmn
	    130 PR "This is digit value "
	    140 PL m
	    150 MIa
	    160 G<30a5
	    170 PL "Done!"
	    180 EN


## Display random numbers, timed

This will display random numbers, with a delay of 1 second between

	    10 RE Display random numbers five times with a delay
	    20 LEg0
	    30 RA 42
	    40 PR "A random number 0..10: " 
	    50 RN a 10
	    60 PL a
	    70 PR "A random number 0..100: " 
	    80 RN a 100
	    90 PL a
	    100 PR "A random number 0..1000: " 
	    110 RN a 1000
	    120 PL a
	    130 PL
	    140 WA 1000
	    150 MIg
	    160 G< 40 g 5
	    180 PL "Done!"
	    180 EN

And here's an updated version of it that uses CAll (GOSUB) routines to 
consolidate some redundancy:

	    10 RE Display random numbers five times with a delay
	    20 LEg0
	    30 RA 42

	    40 LE b 10
	    50 CA 2000

	    60 LE b 100
	    70 CA 2000

	    80 LE b 1000
	    90 CA 2000

	    100 WA 1000
	    110 MIg
	    120 G< 40 g 5
	    130 PL "Done!"
	    140 EN

	    2000 RE Sub: display a random number from 0..(b)
	    2010 PR "A random number 0.." 
	    2020 PR b
	    2030 PR ": "
	    2040 RN a b
	    2050 PL a
	    2060 CR

## CAll/GOSUB example

Just a little to show how CAlls can be done


	10 RE Call examples
	20 LE a8
	30 PR "Value "
	40 PR a
	60 C< 1000 a 10
	70 C> 2000 a 10
	80 C= 3000 a 10

	100 MI a
	110 G< 30 a13
	130 PL "Done."
	140 EN
	
	1000 PL " is less than 10"
	1010 CR

	2000 PL " is greater than 10"
	2010 CR

	3000 PL " is, in fact, 10"
	3010 CR
	


## CAll/GOSUB example number 2

This does the same as the above, but a litle differently

	10 RE do conditionals different this time
	20 LE a8

	30 RE This is the new loop
	40 CA 1000
	50 MI a
	60 G< 30 a 13
	70 PL "Done."
	80 EN

	1000 RE the subroutine is here, and does the compare
	1010 PR "Value " 
	1020 PR a
	1030 G< 2000 a 10
	1040 G> 2100 a 10
	1050 G= 2200 a 10

	2000 PL " is less than 10"
	2010 CR
	2100 PL " is greater than 10"
	2110 CR
	2200 PL " is, in fact, 10"
	2210 CR


## Depleat the Call stack

This one is fun to do on various BASIC implementations.  It basically
recursively calls itself, depleating the number of times a GOSUB
can call itself.  It will eventually fail when the gosub stack is 
saturated. Here's our version of it:

	    10 RE Depleat the CAll stack
	    10 LEa0
	    20 PR "Call stack level "
	    30 PLa
	    40 MIa
	    50 CA20

## IF + CAll examples

Here are some examples using IF statements

	10 RE IF examples
	30 LE g0

	40 PR "Loop number "
	50 PL g

	60 IF g < 5 CA 1000
	70 IF g > 5 CA 1100
	80 IF g = 5 CA 1200

	100 MI g
	110 IF g < 10 GO 40
	120 PL "Done!" 
	130 EN

	1000 PL "  Less than 5";
	1010 CR

	1100 PL "  Greater than 5";
	1110 CR
	
	1200 PL "  Equals than 5";
	1210 CR


## ON + CAll examples

	10 RE ON example
	20 LE g0
	30 PR "Loop number " 
	40 PL g
	50 ON g CA 1000 1100 1200 1300 1400 1500

	60 MI g
	70 IF g < 5 GO 30
	80 PL "Done!" 
	90 EN

	1000 PL "Zero"
	1010 CR

	1100 PL "One"
	1110 CR

	1200 PL "Two"
	1210 CR

	1300 PL "Three"
	1310 CR

	1400 PL "Four"
	1410 CR

	1500 PL "Five"
	1510 CR


## PEEK (EEPROM)

	10 RE Dump out first 100 bytes of EEPROM
	20 LE a0
	30 EE ba
	40 PR b
	50 PR " "
	60 MI a
	70 IF a<100 GO 20
	80 PL "Done!"
	90 EN

## POKE (RAM) and self-modifying RAM

	10 RE Kill this program by nulling out the memory
	20 LE a0
	30 LE d0
	40 PO a d
	50 PL "Done!  Try 'list' now!"
	60 EN

## Turn on autorun (POKE EEPROM) (other config stuff too)

This puts three bytes at the end of the EEPROM which trigger us to
auto-run the program at startup time.  As it is, we load the program
from EEPROM to RAM at startup time automatically.  If the final
three bytes of the EEPROM are 'B', 'L', 1, (or in decimal, 66, 76, 1)
then we will run the program also.

Here is the program to turn on autorun for ATmega168 and ATmega8, with 512 
bytes of EEPROM space:

	10 EO 509 66
	20 EO 510 76
	30 EO 511 1
	40 EN

Note that this can be simplified using sequential EEPROM pokes on a single line:

	10 EO 509 66 76 1
	20 EN

Here is the program to turn on autorun for ATmega328, with 1024 bytes of EEPROM space:

	10 EO 1021 66 76 1
	20 EN

And here is the program to turn on autorun for ATmega1280. ATmega2560,
and for DESKTOP builds, with 4096 bytes of EEPROM space:

	10 EO 4093 66 76 1
	20 EN

For additional configuration stuff, add these numbers to the last byte:

	- 1 - (0x01) Autorun the stored program
	- 32 - (0x20) Add 50ms delay after each newline in list/elist
	- 64 - (0x40) Add 250ms delay after each newline in list/elist
	- 128 - (0x80) Add 500ms delay after each newline in list/elist

A universal version of this can be used if you clear the variables to
their initial state using the 'clear' command.  The EEPROM size is 
stored in variable 'e', so all you need to do is subtract three, then
poke the desired values into the EEPROM like so:

	10 M- a e 3
	20 EO a 66 76 1


## Blink the LED (pin 13) using DigtalWrite and WAit

	10 RE Blink the LED a bit
	20 LE w 200
	30 CA 1000
	40 LE w 400
	50 CA 1000
	60 PL "Done!"
	70 EN
	1000 LE g 0
	1010 DW 13 0
	1020 WA w
	1030 DW 13 1
	1040 WA w
	1050 MI g
	1060 IF g < 5 GO 1010
	1070 CR


## Example of WAit and MS time

	10 RE print out the time in ms, wait for 500ms (1/2 sec) a few times
	20 LE g 0
	30 MS a
	40 PL a
	50 WA 500
	60 MI g
	70 IF g < 5 GO 30
	80 PL "Done!"
	90 EN


## Recreation of the "10 PRINT" BASIC one-liner

This is a BLuB implementation of the maze-generating one-liner as seen in
the publication, "10 PRINT", as found at http://10print.org/

For this, we'll have to get a little creative to get it to work.
We'll do all of the same things, but since BLuB is a bit different
than basic as far as putting things on one line, we'll break it
apart to multiple lines.

	100 RE recreate the maze-generating BASIC one-liner
	110 LE g 0

	200 RE pick 47 or 92: a = 47 + (random( 1 ) * 45)
	210 RN a 1
	220 M* a a 45
	230 M+ a a 47

	300 RE print out the value as ascii '/' or '\'
	310 PC a

	400 RE repeat this 800 times (10 lines)
	410 MI g
	420 IF g < 800 GO 200

	500 PL
	510 PL "Done."
	520 EN

