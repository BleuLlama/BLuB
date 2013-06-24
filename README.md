BLuBasic
========

A small variant of a BASIC style language, simplifying the syntax and
borrowing some concepts from ASM.


# Overview

The focus of this is not to be accurate to a standard BASIC syntax,
but instead use BASIC and its ease of use to be inspiration for a
VERY small programmers shell for use on very low footprint
microcontrollers.

The project provides two methods for building it:

1. unixey desktop.  This target is for building it for runtime on a 
   desktop machine (Mac OSX, MinGW on Windows, Linux) for use of learning
   or for debugging without a microcontroller.  This is accomplished via
   use of the content in the "ardsim" subdirectory.  This uses C++ 
   overloading to provide exact replacements for the ARDUINO interface
   without changing any code in the .INO Arduino source file.

2. Arduino source.  This target is for use for building it for use on
   Arduino devices.  Just double-click the .INO file, and it will open 
   inside of the Arduino IDE.

This does not provide an interface for SD or other storage, and none
is planned.  This is to keep everything small and self-contained.  It 
uses RAM for the primary runtime space, and EEPROM for power-off storage.

It is expected that the serial interface will be used to input programs
to be stored into the EEPROM


--

# Syntax and Command Opcodes

The user-interaction interface for this is serial, and the desktop
interface is a line based text input.

(Line Number) (Opcode) (Optional Operands)

- Line number is an immediate integer base-10 numerical digit.
- Opcode is two uppercase letters
- Operands are lowercase letters for variables
- Operands are integer base-10 digits for immediate numbers
- Operands of multiple digits can be separated with a comma
- Operands of string type start with a double quote to another dquote 

        

# Variables

There are 26 variables, indicated by lowercase letters.
Some variables are preset with values at the beginning of runtime.

- t = 255
- f = 0
- z = 0
- h = 128

The remaining variables are initialized to '0' at start of runtime.


# Interface Commands

These commands are typed in at the BLuB prompt by the user

- mem - display amount of free space in RAM and EEPROM
- help - display a list of commands and version information
- new - clear program memory, initialize variables
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

        IC  (D)             INCHR - LET D = (newly INPUTTED character)
        IL  (D)             same as IC, but ingore to end of line.

    Variable Assignment
        LE  (D) (P)         LET D = P

    Peek and Poke
        PE  (D) (A)         LET D = PEEK( A ) - get from RAM
        PO  (A) (V)         POKE( A, V )      - store V in RAM
        EE  (D) (A)         LET D = PEEK( A ) - get from EEPROM
        EO  (A) (V)         POKE( A, V )      - store V in EEPROM

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
	120 PL
	130 GO 5000
	
	1000 PL " is less than 10"
	1010 CR

	2000 PL " is greater than 10"
	2010 CR

	3000 PL " is, in fact, 10"
	3010 CR
	
	5000 RE do conditionals different this time
	5020 LE a8

	5030 PR "Value " 
	5040 PR a
	5050 CA 8000
	5060 CA 6000
	5070 CA 7000

	5100 MI a
	5110 G< 5030 a 13
	5120 PL "Done."
	5130 EN

	6000 G< 6020 a 10
	6010 CR
	6020 PL " is less than 10"
	6040 CR

	7000 G> 7020 a 10
	7010 CR
	7020 PL " is greater than 10"
	7040 CR

	8000 G= 8020 a 10
	8010 CR
	8020 PL " is in fact than 10"
	8040 CR

	



## Depleat the Call stack

This one is fun to do on various BASIC implementations.  It basically
recursively calls itself, depleating the number of times a GOSUB
can call itself.  Here's our version of it:

	    10 RE Depleat the CAll stack
	    10 LEa0
	    20 PR "Call stack level "
	    30 PLa
	    40 MIa
	    50 CA20


