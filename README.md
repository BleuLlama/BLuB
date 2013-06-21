BLuBasic
========

A (hopefully) very tiny variant of a BASIC style language.

# Overview

The focus of this is not to be accurate to a standard BASIC syntax,
but instead use BASIC and its ease of use to be inspiration for a
VERY small programmers shell for use on very low footprint
microcontrollers.

The project will provide two methods for building it:

1. unixey desktop.  This target is for building it for runtime on a 
   desktop machine (Mac OSX, MinGW on Windows, Linux) for use of learning
   or for debugging without a microcontroller.

2. Arduino source.  This target is for use for building it for use on
   Arduino devices.

It does not rely on SD or Flash storage, to save on space, but
instead will use the RAM for live use, and EEPROM for storage and
persistance between power cycles.

--

# Preliminary Syntax and opcodes

The micro interface for this is serial, and the desktop interface
is a line based text input.

	(Line Number) (Opcode) (Optional Operands)

	Line number is an immediate numerical digit.
	Opcode is two uppercase letters
	Operands are lowercase letters for variables
	Operands are digits for immediate numbers
	Operands of multiple digits can be separated with a comma
	Operands of string type start with a double quote to another dquote 

10 REM fibonacci sequence
20 LET a = 1
30 LET b = 1
35 FOR z = 0 TO 20
40 LET c = a + b
45 PRINT a ; " " ;
50 LET a = b
60 LET b = c
70 NEXT z

	Example (Fib.blb)
	10LDa1			; LET a = 1
	20 LD b 2		; LET b = 1
	30 LD g 0		; LET g = 0
	40 M+ c a b		; LET c = a + b
	40 M+cab		; LET  c = a + b
	50 PLa			; PRINT A
	60 PL" "		; PRINT " "
	70 LD ab		; LET a = b
	80 LD bc		; LET b = c
	90 MI g			; LET g = g + 1
	90 M+ g g 1		; LET g = g + 1
	100 JL g 20 40		; Jump if g is less than 20 to 40
	

The operations are a mix between BASIC and a sort-of Assembler

There are 26 variables, indicated by lowercase letters.
Some varialbes are preset with values at the beginning of runtime.
These are the only ones guaranteed to be set.
	t = 255
	f = 0
	z = 0
	h = 128

Commands:
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



Line Operations
    System
	NP			Do nothing
	ST			Stop
	RE			remark (comment) to end of line

    Text
	PP  (P)			print parameter or string
	PL  (P)			println parameter or string

    Variable
	LD  (D) (P)		D = P

	LR  (D) (P)		D = ram[ P ]
	LE  (D) (P)		D = EEPROM[ P ]
	SR  (D) (P)		ram[ P ] = D
	SE  (D) (P)		EEPROM[ P ] = D

    Math
	M+  (D) (P) (Q)		D = P + Q
	M-  (D) (P) (Q)		D = P - Q
	M/  (D) (P) (Q)		D = P / Q
	M*  (D) (P) (Q)		D = P * Q

	MI  (D)			D = D + 1
	MD  (D)			D = D - 1

    Bitwise operations
	M<  (D)	(P)		D = D << P  (left shift)
	M>  (D)	(P)		D = D >> P  (right shift)
	M&  (D) (P)		D = D & P   (bit mask)
	M|  (D) (P)		D = D | P   (bit set)
	M!  (D)			D = ~D      (invert bits)
	
    Jumps
	JR  (D)			goto D
	JL  (D) (P) (Q)		goto D if P < Q
	JG  (D) (P) (Q)		goto D if P > Q
	JE  (D) (P) (Q)		goto D if P == Q

	GS  (D)			gosub D
	GL  (D) (P) (Q)		gosub D if P < Q
	GG  (D) (P) (Q)		gosub D if P > Q
	GE  (D) (P) (Q)		gosub D if P == Q
	RT			return from subroutine

    Digital IO
	AW  (D) (V)		analog write to Destination pin, value V
	DW  (D) (V)		digital write to Destination pin, value V
	AR  (D) (P)		analog read to D the value of pin P
	DR  (D) (P)		digital read to D the value of pin P

