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


## Example BASIC program (fibonacci sequence)

	    10 REM fibonacci sequence
	    20 LET a = 1
	    30 LET b = 1
	    40 FOR z = 0 TO 20
	    50 LET c = a + b
	    60 PRINT a ; " " ;
	    70 LET a = b
	    80 LET b = c
	    90 NEXT z
	    100 END


## Same example in BLuB:

	    Code                Explanation
	    10 RE Fibonacci Sequence
	    20LEa1		; LET a = 1
	    30 LE b 2		; LET b = 1
	    40 LE g 0		; LET g = 0
	    50 M+cab		; LET  c = a + b
	    60 PPa		; PRINT A
	    70 PP" "		; PRINT " "
	    80 LE ab		; LET a = b
	    90 LE bc		; LET b = c
	    100 MI g		; LET g = g + 1
	    110 G< 40 g 20	; IF (G < 20 ) GOTO 40
	    120 EN
	

# Variables

There are 26 variables, indicated by lowercase letters.
Some varialbes are preset with values at the beginning of runtime.
	t = 255
	f = 0
	z = 0
	h = 128

The remaining variables are initialized to '0' at start of runtime.

--

# Interface Commands:
	mem	display amount of free space in RAM and EEPROM
	help	display a list of commands and version information

	new 	clear program memory, initialize variables
	list	display the program loaded into memory

	run	run the program in memory
	tron	turn program runtime trace on
	troff	turn program runtime trace off

	enew	clear EEPROM
	elist	display the program stored in EEPROM
	eload	load the program from EEPROM to memory
	esave	save the program from memory to EEPROM


# Program Opcodes:

    System
	RE			REM - add a comment
	EN			END - end runtime

    Text IO
	PP  (P)			PRINT ; - print a parameter or string
	PL  (P)			PRINT   - output a parameter or string (newline)

	IC  (D)			INCHR - LET D = (newly INPUTTED character)
	IL  (D)			same as IC, but ingore to end of line.

    Variable Assignment
	LE  (D) (P)		LET D = P

    Peek and Poke
	PE  (D) (A)		LET D = PEEK( A ) - get from RAM
	PO  (A) (V)		POKE( A, V )      - store V in RAM
	EE  (D) (A)		LET D = PEEK( A ) - get from EEPROM
	EO  (A) (V)		POKE( A, V )      - store V in EEPROM

    Math
	M+  (D) (P) (Q)		LET D = P + Q
	M-  (D) (P) (Q)		LET D = P - Q
	M/  (D) (P) (Q)		LET D = P / Q
	M*  (D) (P) (Q)		LET D = P * Q

	MI  (D)			LET D = D + 1
	MD  (D)			LET D = D - 1

    Bitwise operations
	M<  (D)	(P)		LET D = D << P  (left shift)
	M>  (D)	(P)		LET D = D >> P  (right shift)
	M&  (D) (P)		LET D = D & P   (bit mask)
	M|  (D) (P)		LET D = D | P   (bit set)
	M!  (D)			LET D = ~D      (invert bits)
	
    GOTO
	GO  (D)			GOTO D
	G<  (D) (P) (Q)		IF ( P > Q ) THEN GOTO D
	G>  (D) (P) (Q)		IF ( P < Q ) THEN GOTO D
	G=  (D) (P) (Q)		IF ( P = Q ) THEN GOTO D

    GOSUB/CALL
	CA  (D)			CALL D
	C<  (D) (P) (Q)		IF ( P > Q ) THEN CALL D
	C>  (D) (P) (Q)		IF ( P < Q ) THEN CALL D
	C=  (D) (P) (Q)		IF ( P = Q ) THEN CALL D
	CR			RETURN

    Digital IO
	AW  (P) (V)		analog write the value V to pin P
	DW  (P) (V)		digital write the value V to pin P
	AR  (D) (P)		analog read the value of pin P to varable D
	DR  (D) (P)		digital read the value of pin P to varable D

