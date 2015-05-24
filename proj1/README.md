##1. Purpose
This project is intended to help you understand the instructions of a very
simple assembly language and how to assemble programs into machine language.

##2.Problem
This project has three parts.  In the first part, you will write a program to
take an assembly-language program and produce the corresponding machine
language.  In the second part, you will write a behavioral simulator for the
resulting machine code.  In the third part, you will write a short
assembly-language program to multiply two numbers.

##3. LC3101 Instruction-Set Architecture
For this project, you will be developing a simulator and assembler for the
LC3101 (Little Computer, used in CDA 3101). The LC3101 is very simple, but
it is general enough to solve complex problems. For this project, you will
only need to know the instruction set and instruction format of the LC3101.

The LC3101 is an 8-register, 32-bit computer.  All addresses are
word-addresses (unline MIPS which is byte-addressed).  The LC3101 has 65536
words of memory.  By assembly-language convention, register 0 will always
contain 0 (i.e. the machine will not enforce this, but no assembly-language
program should ever change register 0 from its initial value of 0).

There are 3 instruction formats (bit 0 is the least-significant bit).  Bits
31-25 are unused for all instructions, and should always be 0.


> R-type instructions (add, nand):
>   bits 24-22: opcode
>    bits 21-19: reg A
>    bits 18-16: reg B
>    bits 15-3:  unused (should all be 0)
>    bits 2-0:   destReg

> I-type instructions (lw, sw, beq):
>    bits 24-22: opcode
>    bits 21-19: reg A
>    bits 18-16: reg B
>    bits 15-0:  offsetField (16-bit, range of -32768 to 32767)

> O-type instructions (halt, noop):
>    bits 24-22: opcode
>    bits 21-0:  unused (should all be 0)

```
-------------------------------------------------------------------------------
Table 1: Description of Machine Instructions
-------------------------------------------------------------------------------
Assembly language 	Opcode in binary		Action
name for instruction	(bits 24, 23, 22)
-------------------------------------------------------------------------------
add (R-type format)	000 			add contents of regA with
						contents of regB, store
						results in destReg.

nand (R-type format)	001			nand contents of regA with
						contents of regB, store
						results in destReg.

lw (I-type format)	010			load regB from memory. Memory
						address is formed by adding
						offsetField with the contents of
						regA.

sw (I-type format)	011			store regB into memory. Memory
						address is formed by adding
						offsetField with the contents of
						regA.

beq (I-type format)	100			if the contents of regA and
						regB are the same, then branch
						to the address PC+1+offsetField,
						where PC is the address of the
						beq instruction.

cmov (R-type)	 	101			copy the value regA into destReg
						if the contents of regB != 0
						

halt (O-type format)	110			increment the PC (as with all
						instructions), then halt the
						machine (let the simulator
						notice that the machine
						halted).

noop (O-type format)	111			do nothing except increment PC.
```

##4. LC3101 Assembly Language and Assembler (40%)
The first part of this project is to write a program to take an
assembly-language program and translate it into machine language. You will
translate assembly-language names for instructions, such as beq, into their
numeric equivalent (e.g. 100), and you will translate symbolic names for
addresses into numeric values. The final output will be a series of 32-bit
instructions (instruction bits 31-25 are always 0).

The format for a line of assembly code is: `label  instruction  field0  field1  field2  comments`

The leftmost field on a line is the label field.  Valid labels contain a
maximum of 6 characters and can consist of letters and numbers (but must start
with a letter). The label is optional (the white space following the label
field is required).  Labels make it much easier to write assembly-language
programs, since otherwise you would need to modify all address fields each time
you added a line to your assembly-language program!

After the optional label is white space which consists of any number of space
or tab characters.  The writespace is followed by the instruction field,
where the instruction can be any of the assembly-language instruction names
listed in the above table.  After more white space comes a series of fields.
All fields are given as decimal numbers or labels.  The number of fields
depends on the instruction, and unused fields should be ignored (treat them
like comments).

R-type instructions (add, nand) instructions require 3 fields: field0
is regA, field1 is regB, and field2 is destReg.

I-type instructions (lw, sw, beq) require 3 fields: field0 is regA, field1
is regB, and field2 is either a numeric value for offsetField or a symbolic
address.  Numeric offsetFields can be positive or negative; symbolic
addresses are discussed below.

O-type instructions (noop and halt) require no fields.

Symbolic addresses refer to labels.  For lw or sw instructions, the assembler
should compute offsetField to be equal to the address of the label.  This could
be used with a zero base register to refer to the label, or could be used with
a non-zero base register to index into an array starting at the label.  For beq
instructions, the assembler should translate the label into the numeric
offsetField needed to branch to that label.

After the last used field comes more white space, then any comments.  The
comment field ends at the end of a line.  Comments are vital to creating
understandable assembly-language programs, because the instructions themselves
are rather cryptic.

In addition to LC3101 instructions, an assembly-language program may contain
directions for the assembler. The only assembler directive we will use is .fill
(note the leading period). .fill tells the assembler to put a number into the
place where the instruction would normally be stored. .fill instructions use
one field, which can be either a numeric value or a symbolic address.  For
example, ".fill 32" puts the value 32 where the instruction would normally be
stored.  .fill with a symbolic address will store the address of the label.
In the example below, ".fill start" will store the value 2, because the label
"start" is at address 2.

The assembler should make two passes over the assembly-language program. In the
first pass, it will calculate the address for every symbolic label.  Assume
that the first instruction is at address 0.  In the second pass, it will
generate a machine-language instruction (in decimal) for each line of assembly
language.  For example, here is an assembly-language program (that counts down
from 5, stopping when it hits 0).
```
	lw	0	1	five	load reg1 with 5 (uses symbolic address)
	lw	1	2	3	load reg2 with -1 (uses numeric address)
start	add	1	2	1	decrement reg1
	beq	0	1	2	goto end of program when reg1==0
	beq	0	0	start	go back to the beginning of the loop
	noop
done	halt				end of program
five	.fill	5
neg1	.fill	-1
stAddr	.fill	start			will contain the address of start (2)
```
And here is the corresponding machine language:
```
(address 0): 8454151 (hex 0x810007)
(address 1): 9043971 (hex 0x8a0003)
(address 2): 655361 (hex 0xa0001)
(address 3): 16842754 (hex 0x1010002)
(address 4): 16842749 (hex 0x100fffd)
(address 5): 29360128 (hex 0x1c00000)
(address 6): 25165824 (hex 0x1800000)
(address 7): 5 (hex 0x5)
(address 8): -1 (hex 0xffffffff)
(address 9): 2 (hex 0x2)
```
Be sure you understand how the above assembly-language program got translated
to machine language.

Since your programs will always start at address 0, your program should only
output the contents, not the addresses.

```
8454151
9043971
655361
16842754
16842749
29360128
25165824
5
-1
2
```
