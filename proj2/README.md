#1. Purpose

This project is intended to help you understand in detail how a pipelined
implementation works.  You will write a cycle-accurate behavioral simulator for
a pipelined implementation of the LC3101, complete with data forwarding and
simple branch prediction.

#2. LC3101 Pipelined Implementation
##2.1. Datapath
For this project we will use the datapath from  Patterson and Hennessy. 
Of course, since the MIPS and LC3101 architectures are slightly different,
we will have to make a few minor changes to the book's datapath.
    1) Instead of a "4" input in the PC's adder, we will use a "1", since the
	LC3101 is word-addressed instead of byte-addressed.
    2) The instruction bit fields have to be modified to suit the LC3101's
	instruction-set architecture.
    3) The "shift left 2" component is not necessary, since both offsetField
	for branches and the PC use word-addressing.
The main difference between Project 2 and the pipelining done in the book is
that we will add a pipeline register AFTER the write-back stage (the WBEND
pipeline register).  This will be used to simplify data forwarding so that the
register file does not have to do any internal forwarding.

To follow the pipelining done in the textbook as closely as possible, we will
use the MIPS clocking scheme (e.g. register file and memory writes require the
data to be present for the whole cycle).

##2.2. Memory
Note in the typedef of stateType below that there are two memories: instrMem
and dataMem.  When the program starts, read the machine-code file into BOTH
instrMem and dataMem (i.e. they'll have the same contents in the beginning).
During execution, read instructions from instrMem and perform load/stores using
dataMem.  That is, instrMem will never change after the program starts, but
dataMem will change.  (In a real machine, these two memories would be an
instruction and data cache, and they would be kept consistent.)

##2.3. Pipeline Registers
To simplify the project and make the output formats uniform, you must use the
following structures WITHOUT MODIFICATION to hold pipeline register contents.
Note that the instruction gets passed down the pipeline in its entirety.
```c
	#define NUMMEMORY 65536 /* maximum number of data words in memory */
	#define NUMREGS 8 /* number of machine registers */

	#define ADD 0
	#define NAND 1
	#define LW 2
	#define SW 3
	#define BEQ 4
	#define CMOV 5  
	#define HALT 6
	#define NOOP 7

	#define NOOPINSTRUCTION 0x1c00000

	typedef struct IFIDStruct {
	    int instr;
	    int pcPlus1;
	} IFIDType;

	typedef struct IDEXStruct {
	    int instr;
	    int pcPlus1;
	    int readRegA;
	    int readRegB;
	    int offset;
	} IDEXType;

	typedef struct EXMEMStruct {
	    int instr;
	    int branchTarget;
	    int aluResult;
	    int readRegB;
	} EXMEMType;

	typedef struct MEMWBStruct {
	    int instr;
	    int writeData;
	} MEMWBType;

	typedef struct WBENDStruct {
	    int instr;
	    int writeData;
	} WBENDType;

	typedef struct stateStruct {
	    int pc;
	    int instrMem[NUMMEMORY];
	    int dataMem[NUMMEMORY];
	    int reg[NUMREGS];
	    int numMemory;
	    IFIDType IFID;
	    IDEXType IDEX;
	    EXMEMType EXMEM;
	    MEMWBType MEMWB;
	    WBENDType WBEND;
	    int cycles; /* number of cycles run so far */
	} stateType;
```
