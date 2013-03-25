#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

// OPCODES
#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define CMOV 5  
#define HALT 6
#define NOOP 7
#define NOOPINSTRUCTION 0x1c00000

/*****************************************************************************/
// IF/ID
typedef struct IFIDStruct {
    int instr;
    int pcPlus1;
} IFIDType;

// ID/EX
typedef struct IDEXStruct {
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int offset;
} IDEXType;

// EX/MEM
typedef struct EXMEMStruct {
    int instr;
    int branchTarget;
    int aluResult;
    int readRegB;
} EXMEMType;

// MEM/WB
typedef struct MEMWBStruct {
    int instr;
    int writeData;
} MEMWBType;

// WB/END
typedef struct WBENDStruct {
    int instr;
    int writeData;
} WBENDType;

// Actual state container which holds pretty much everything.
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

/*****************************************************************************/
// The following are functions that have been given.
int
field0(int instruction)
{
    return( (instruction>>19) & 0x7);
}

int
field1(int instruction)
{
    return( (instruction>>16) & 0x7);
}

int
field2(int instruction)
{
    return(instruction & 0xFFFF);
}

int opcode(int instruction)
{
    return(instruction>>22);
}

void
printInstruction(int instr)
{
    char opcodeString[10];
    if (opcode(instr) == ADD) {
    strcpy(opcodeString, "add");
    } else if (opcode(instr) == NAND) {
    strcpy(opcodeString, "nand");
    } else if (opcode(instr) == LW) {
    strcpy(opcodeString, "lw");
    } else if (opcode(instr) == SW) {
    strcpy(opcodeString, "sw");
    } else if (opcode(instr) == BEQ) {
    strcpy(opcodeString, "beq");
    } else if (opcode(instr) == CMOV) {
    strcpy(opcodeString, "cmov");
    } else if (opcode(instr) == HALT) {
    strcpy(opcodeString, "halt");
    } else if (opcode(instr) == NOOP) {
    strcpy(opcodeString, "noop");
    } else {
    strcpy(opcodeString, "data");
    }

    printf(	"%s %d %d %d\n", 
    		opcodeString, 
    		field0(instr), 
    		field1(instr),
   			field2(instr) );
}


void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("\tIFID:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
    printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
    printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
    printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
    printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
    printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->WBEND.instr);
    printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

/*****************************************************************************/

void initialize( stateType* );
void loadInstructions( stateType*, const char* );

/*****************************************************************************/


int main(int argc, const char *argv[])
{
	if( argc != 2 ) {
		fprintf( stderr, 
				 "ERROR: Usage: %s <machine-code file>\n", 
				 argv[0] );
	}

	// Initialize State
	stateType state;
	initialize( &state );

	// Load instructions.
	loadInstructions( &state, argv[1] );

}

/**
 	 * Initializes the state of this Processor.
 */
void initialize( stateType* statePtr )
{
	statePtr->pc = 0;
	statePtr->cycles = 0;

	int i;
	for( i = 0; i < NUMREGS; i++ ) {
		statePtr->reg[i] = 0;
	}

	// Init IF/ID
	statePtr->IFID.instr = NOOPINSTRUCTION;

	// Init ID/EX
	statePtr->IDEX.instr = NOOPINSTRUCTION;

	// Init EX/MEM
	statePtr->EXMEM.instr = NOOPINSTRUCTION;

	// Init MEM/WB
	statePtr->MEMWB.instr = NOOPINSTRUCTION;

	// Init WB/END
	statePtr->WBEND.instr = NOOPINSTRUCTION;
}

/**
 	 * Read an assembly file.
 */
void loadInstructions( stateType* statePtr, const char *filename )
{
	// Attempt to open file.
	FILE *infile = fopen( filename, "r" );
	char line[MAXLINELENGTH];

	if( infile == NULL ) {
		fprintf( stderr, 
				 "ERROR: Cannot open file %s\n", 
				 filename );
		perror("fopen");
		exit(1);
	}


	for( statePtr->numMemory = 0; 
		 fgets(line, MAXLINELENGTH, infile) != NULL;
		 statePtr->numMemory++ )
	{
		if( sscanf( line, "%d", &statePtr->instrMem[statePtr->numMemory]) != 1)
		{
			fprintf( stderr, 
					 "ERROR: Failed to read %d\n", 
					 statePtr->instrMem[statePtr->numMemory] );
			exit(1);
		}
		else
		{
			statePtr->dataMem[statePtr->numMemory] = statePtr->instrMem[statePtr->numMemory];
			printf( "memory[%d]=%d\n",
					statePtr->numMemory,
					statePtr->instrMem[statePtr->numMemory] );
		}

		// Exceeding memory.
		if(statePtr->numMemory >= NUMMEMORY) {
			fprintf( stderr, 
					 "ERROR: Maximum memory exceeded. Max = %d\n", 
					 NUMMEMORY );
			exit(1);
		}
	}

	// print instruction memory.
	printf( "%d memory words\n", statePtr->numMemory );
	printf( "\tinstruction memory:\n" );
	int i;

	for(i = 0; i < statePtr->numMemory; i++)
	{
		printf( "\t\tinstrMem[ %d ] ", i );
		printInstruction( statePtr->instrMem[i] );
	}
}