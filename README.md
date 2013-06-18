BLuBasic
========

A very tiny variant of a BASIC style language.

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
	50 ??a			; PRINT A;
	60 ??" "		; PRINT " ";
	70 LD ab		; LET a = b
	80 LD bc		; LET b = c
	90 MI g			; LET g = g + 1
	90 M+ g g 1		; LET g = g + 1
	100 JL g 20 40		; Jump if g is less than 20 to 40
	

Addresses are added sequentially.  Insert/renumbering will be an
option in the future.

The operations are a mix between BASIC and a sort-of Assembler

There are 26 variables, indicated by lowercase letters.


Operations:
	Opcode	Name	operands	Description
	N	noop	-		do nothing
	A	add	V0 V1 V2	V0 = V1 + V2
	S	sub	V0 V1 V2	V0 = V1 - V2
	M	mult	V0 V1 V2	V0 = V1 * V2
	D 	divide	V0 V1 V2	V0 = V1 / V2
	E	equals	V0 V1		V0 = V1
	I	inc	V0		V0 = V0 + 1
	D	dec	V0		V0 = V0 - 1
	J	jump	V0		Jump to address V0
	DJNE	djnz	V0 V1		Jump to address V0 if V1 is not zero
	Is	Set	V0 (val)	Set V0 with immediate value

	?	Print	V0		Print the value in V0 (decimal)
	? 	Print   "TXT"		Print out the text
	? 	Print   N		Print out a newline

	W	Write	V0 V1		DigitalWrite( pin V0 from V1 )
	X	AWrite	V0 V1		AnalogWrite( pin V0 from V1 )
	R	Read	V0 V1		DigitalRead( pin V0 into V1 )
	S	ARead	V0 V1		AnalogRead( pin V0 into V1 )


	10 A = DigitalRead( 3 )
	20 B = AnalogRead( 3 )
	30 C = A * 255 / 1024
	40 Print "c is "; c
	50 AnalogWrite( 11, C )
	60 AnalogWrite( 10, 200 )

	Tokenized basicish

	10 REM Program to do example stuff
	20 DRa'3
 	30 ARb'3
	40 M*CA'255
	50 M/C'1024
	60 ??"c is "
	70 ??c
	80 ??"\n"
	90 AW'11c
	100 AW'10'200

	opcodes are 2 characters
	110 IGb'100'40
	120 GO'20
	

	parameters (letters in the description) can be:
		lowercase letter (variable name)
		immediate value 

	immediate values start with a single quote "'", and go 
	until the next non-numeric digit.  0..9
	Values are integer

	IG	if A greater than B, goto C

	IF Greater A B, goto C

	math operations start with 'M'.
