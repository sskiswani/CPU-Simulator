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

/*****************************************************************************//*****************************************************************************/

void initialize( stateType* );
void loadInstructions( stateType*, const char* );
void fetch( const stateType, stateType* );
void decode( const stateType, stateType* );
void execute( const stateType, stateType* );
void memory( const stateType, stateType* );
void writeBack( const stateType, stateType* );
int resolveHazard( const stateType, stateType* ); // TODO: eliminate.
void forward( const stateType, stateType* );
int detectAndStall( const stateType );

/*****************************************************************************//*****************************************************************************/


int main(int argc, const char *argv[])
{
	// DEBUG: maxCycles stops the CPU after a certain amount of cycles.
	int maxCycles = 0;

	if( argc == 3) {
		sscanf( argv[2], "%d", &maxCycles );

		fprintf( stderr, 
				 "**** CYCLE CAP: %d ***\n", 
				 maxCycles );
	}
	else if( argc != 2 ) {
		fprintf( stderr, 
				 "ERROR: Usage: %s <machine-code file>\n", 
				 argv[0] );
	}

	// Initialize State
	stateType state, newState;
	initialize( &state );
	loadInstructions( &state, argv[1] );

	while(1)
	{
		printState( &state );

		/* Simulation Complete. */
		if( opcode(state.MEMWB.instr) == HALT ) {
			printf("Machine halted.\n");
			printf("total of %d cycles executed\n", state.cycles);
			exit(0);
		}

		newState = state;
		newState.cycles++;

		//////////////////////////////////////////////////////////////
		// DEBUG: Cycle Cap.
		if( maxCycles != 0 
			&& state.cycles > maxCycles )
		{
			fprintf(stderr, "***FORCED HALT***.\n");
			exit(0);
		}
		//////////////////////////////////////////////////////////////

		/* --------------------- IF stage --------------------- */
		fetch( state, &newState );
		/* --------------------- ID stage --------------------- */
		decode( state, &newState );
		/* --------------------- EX stage --------------------- */
		execute( state, &newState );
		/* --------------------- MEM stage -------------------- */
		memory( state, &newState );
		/* --------------------- WB stage --------------------- */
		writeBack( state, &newState );

		// DETECT AND STALL
		if( detectAndStall( state ) )
		{
			fprintf(stderr, "Stalling...\n");
			newState.pc--;
			newState.IFID = state.IFID;
			newState.IDEX.instr = NOOPINSTRUCTION;
		}

		state = newState; // conclusion.
	}

}

/******************************************************************************
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

/*****************************************************************************//*****************************************************************************/

/*
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


	// Load instructions.
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

/*****************************************************************************//*****************************************************************************/
/*****************************************************************************//*****************************************************************************/

/*
 	* Instruction Fetch Stage.
 	*
 	* INPUT:
    *
 	* OUTPUT:
 	* 		IFID.instr;
    * 		IFID.pcPlus1;
 */
void fetch( const stateType state, stateType* newStatePtr ) 
{
	IFIDType output;

	// Update PC.
	newStatePtr->pc = state.pc + 1;

	// Update Pipeline Register
	output.instr = state.instrMem[state.pc];
	output.pcPlus1 = state.pc + 1;

	// Update State.
	newStatePtr->IFID = output;
}

/*****************************************************************************//*****************************************************************************/

/*
 	* Instruction Decode Stage.
 	*
 	* INPUT:
 	* 		IFID.instr;
    * 		IFID.pcPlus1;
    *
 	* OUTPUT:
 	* 		IDEX.instr;
    * 		IDEX.pcPlus1;
    * 		IDEX.readRegA;
    * 		IDEX.readRegB;
    * 		IDEX.offset;
 */
void decode( const stateType state, stateType* newStatePtr )
{
	IFIDType input = state.IFID;
	IDEXType output;

	// Update Pipeline Register.
	output.instr = input.instr;
	output.pcPlus1 = input.pcPlus1;

	output.readRegA = state.reg[ field0(input.instr) ];
	output.readRegB = state.reg[ field1(input.instr) ];
	output.offset = field2( input.instr );

	// Update State.
	newStatePtr->IDEX = output;
}

/*****************************************************************************//*****************************************************************************/

/*
 	* Execute Stage.
 	*
 	* INPUT:
 	* 		IDEX.instr;
    * 		IDEX.pcPlus1;
    * 		IDEX.readRegA;
    * 		IDEX.readRegB;
    * 		IDEX.offset;
    *
 	* OUTPUT:
 	* 		EXMEM.instr;
    * 		EXMEM.branchTarget;
    * 		EXMEM.aluResult;
    * 		EXMEM.readRegB;
 */
void execute( const stateType state, stateType* newStatePtr )
{
	IDEXType input = state.IDEX;
	EXMEMType output;

	// Pass the instruction through first,
	// in order to allow cmov to fill with a
	// noop.
	output.instr = input.instr;

	// shorthand.
	int regA = input.readRegA;
	int regB = input.readRegB;
	int offset = input.offset;



	// Calculate aluResult.
	int aluResult;

	switch( opcode(state.IDEX.instr) )
	{
		/*****************************************/
		case ADD:
			aluResult = regA + regB;
			break;
		/*****************************************/
		case NAND:
			aluResult = ~(regA & regB);
			break;
		/*****************************************/
		case CMOV:
		/*
			* CMOV is treated as faithfully as an R-Type instr. as it
			* possibly can; that means that the aluResult (which 
			* becomes the writeData of R-Type instr.) will be regA if
			* the condition is satisfied. In order to prevent any sort
			* of special cases and/or modifications to the given stateType
			* struct, a NOOP is sent to EXMEM, since this is what a CMOV
			* reduces to in the event that it does not modify the contents
			* of the destReg.
		*/ 
			if( regB != 0 ) {
				aluResult = regA;
			}
			else {
				newStatePtr->EXMEM.instr = NOOPINSTRUCTION;
			}
			break;
		/*****************************************/
		case LW:
		case SW:
			// Calculate the offset field.
			aluResult = regA + offset;
			break;
		/*****************************************/
		case BEQ:
			// Check equality.
			aluResult = (regA == regB);
			break;
		/*****************************************/
		case HALT:
		case NOOP:
			// do nothing.
			break;
		/*****************************************/
		default: 
		// Error.
			fprintf( stderr, "ERROR: Instruction not recognized:" );
			printInstruction(state.IDEX.instr);
			exit(1);
			break;
		/*****************************************/
	}

	// Update State.
	newStatePtr->EXMEM = output;
}

/*****************************************************************************//*****************************************************************************/

/*
 	* Memory Operations Stage.
 	*
 	* INPUT:
 	* 		EXMEM.instr;
    * 		EXMEM.branchTarget;
    * 		EXMEM.aluResult;
    * 		EXMEM.readRegB;
    *
 	* OUTPUT:
 	*		MEMWB.instr;
 	*		MEMWB.writeData;
 	*	
 */
void memory( const stateType state, stateType* newStatePtr )
{
	EXMEMType input = state.EXMEM;
	MEMWBType output;

	output.instr = input.instr;

	switch( opcode(input.instr) )
	{
		case ADD:
		case NAND:
		case CMOV:
			// Pass through the ALU result for WriteBack.
			output.writeData = input.aluResult;
			break;
		case LW:
			// load word from dataMem
			output.writeData = state.dataMem[ input.aluResult ];
			break;
		case SW:
			// store word into dataMem
			newStatePtr->dataMem[ input.aluResult ] = input.readRegB;
			break;
		case BEQ:
		case HALT:
		case NOOP:
			// do nothing.
			break;
	}

	// Update Pipeline Register.
	newStatePtr->MEMWB = output;
}

/*****************************************************************************//*****************************************************************************/

/*
 	* Writeback Stage.
 	*
 	* INPUT:
 	*		MEMWB.instr;
 	*		MEMWB.writeData;
    *
 	* OUTPUT:
 	*		WBEND.instr;
 	*		WBEND.writeData;
 */
void writeBack( const stateType state, stateType* newStatePtr )
{
	// Data to be written.
	int writeData = state.MEMWB.writeData;

	// For I-Type instructions.
	int regB = field1(state.MEMWB.instr);

	// For R-Type instructions.
	int destReg = field2(state.MEMWB.instr);

	switch( opcode(state.MEMWB.instr) )
	{
		/*****************************************/
		case ADD:
		case NAND:
		case CMOV:
			// Write Data to destReg.
			newStatePtr->reg[destReg] = writeData;
			break;
		/*****************************************/
		case LW:
			// Write Data to regB.
			newStatePtr->reg[regB] = writeData;
			break;
		/*****************************************/
		case NOOP:
		case HALT:
		case BEQ:
		case SW:
			break;
		/*****************************************/
	}

	// Update Pipeline Register.
	newStatePtr->WBEND.instr = state.MEMWB.instr;
	newStatePtr->WBEND.writeData = state.MEMWB.writeData;
}

/*****************************************************************************//*****************************************************************************/


/*
	* 	Hazard resolver. Analyzes previous state for any data hazards
	* 	before any operations are done. This is neccessary in order to
	* 	stall the processor for operations that cannot be handled by
	* 	the time that they are needed.
	*
	* 	=> returns 1 if a stall is required.
	*	=> returns 0 if no stall is required.
*/
int resolveHazard( const stateType state, stateType* newStatePtr ) 
{
	// Sanitize results so that the Execute stage can operate
	// with the proper values. regA and regB contents are used
	// universally with all instructions. If the IDEX.readReg#
	// values are updated, then execute will operate with the 
	// proper values.
	int regA = field0(state.IDEX.instr);
	int regB = field1(state.IDEX.instr);


	/*
		*	Pipeline registers are checked for data hazards
		*	in order of increasing recency in order to forward
		*	the most up-to-date values to the IDEX register.
	*/
	
	// Check WBEND for hazards.
	switch( opcode(state.WBEND.instr) )
	{
		/*****************************************/
		case ADD:
		case NAND:
		case CMOV:
		/*
			* These instructions are simple because the updated values
			* are in the writeData register and their target is specified
			* by the destReg of the R-type instr. at this stage.
		*/
			if( regA == field2(state.WBEND.instr) ) {
				newStatePtr->IDEX.readRegA = state.WBEND.writeData;
			}
			if( regB == field2(state.WBEND.instr) ) {
				newStatePtr->IDEX.readRegB = state.WBEND.writeData;
			}
			break;
		/*****************************************/
		case LW: 
		/*
			* In the WB/END pipeline register, the value specified by
			* field1 has already been loaded, and we must simply forward
			* this value to the IDEX pipeline register.
		*/
			if( regA == field1(state.WBEND.instr) ) {
				newStatePtr->IDEX.readRegA = state.WBEND.writeData;
			}
			if( regB == field1(state.WBEND.instr) ) {
				newStatePtr->IDEX.readRegB = state.WBEND.writeData;
			}
			break;
		/*****************************************/
		case BEQ:
		case SW:
		case NOOP:
		case HALT:
			// No action neccessary.
			break;
		/*****************************************/
	}


	// Check MEM/WB for hazards.
	switch( opcode(state.MEMWB.instr) )
	{
		/*****************************************/
		case ADD:
		case NAND:
		case CMOV:
		/*
			* These instructions are simple because the updated values
			* are in the writeData register and their target is specified
			* by the destReg of the R-type instr. at this stage.
		*/
			if( regA == field2(state.MEMWB.instr) ) 
			{
				newStatePtr->IDEX.readRegA = state.MEMWB.writeData;
			}
			if( regB == field2(state.MEMWB.instr) ) 
			{
				newStatePtr->IDEX.readRegB = state.MEMWB.writeData;
			}
			break;
		///////////////////////////////////////////
		case LW: 
		/*
			* In the MEM/WB pipeline register, the value specified by
			* field1 has already been loaded, and we must simply forward
			* this value to the IDEX pipeline register.
		*/
			if( regA == field1(state.MEMWB.instr) ) 
			{
				newStatePtr->IDEX.readRegA = state.MEMWB.writeData;
			}
			if( regB == field1(state.MEMWB.instr) ) 
			{
				newStatePtr->IDEX.readRegB = state.MEMWB.writeData;
			}
			break;
		/*****************************************/
		case BEQ:
		case SW:
		case NOOP:
		case HALT:
			// No action neccessary.
			break;
		/*****************************************/
	}


	// Check EX/MEM for hazards.
	switch( opcode(state.EXMEM.instr) )
	{
		/*****************************************/
		case ADD:
		case NAND:
		case CMOV:
		/*
			* Compare the input regA and regB to the destReg of the instr.
			* in EXMEM since it's (in this case) an R-Type and its value will 
			* be waiting in aluResult. This includes CMOV since this will
			* actually be a NOOP if CMOV does not modify its destReg.
		*/
			if( regA == field2(state.EXMEM.instr) ) {
				newStatePtr->IDEX.readRegA = state.EXMEM.aluResult;
			}
			if( regB == field2(state.EXMEM.instr) ) {
				newStatePtr->IDEX.readRegB = state.EXMEM.aluResult;
			}
			break;
		/*****************************************/
		case LW:
		/*
			* If a LW is dected in the EXMEM pipeline, then the CPU has only
			* just calculated the offset for this instr and has not yet been
			* loaded from memory. In this case the only resolution is to stall.
		*/
			if( regA == field1(state.EXMEM.instr)
				|| regB == field1(state.EXMEM.instr) )
			{
				return 1;
			}
			break;
		/*****************************************/
		case SW:
		case BEQ:
		case NOOP:
		case HALT:
			// No action neccessary.
			break;
		/*****************************************/
	}


 	// no stall necessary.
	return 0;
}

int detectAndStall( const stateType state )
{
	// Figure out if IDEX has a value that depends on the result of a
	// LW op that isn't prepared.
	int regA = field0(state.IDEX.instr);
	int regB = field1(state.IDEX.instr);

	if( opcode(state.EXMEM.instr) == LW ) {
		return ( regA == field1(state.EXMEM.instr) 
				 || regB == field1(state.EXMEM.instr) );
	}
	else {
		return 0;
	}
}


void forward( const stateType state, stateType* newStatePtr )
{
	// Find the registers we're dealing with
	int regA = field1( state.IDEX.instr );
	int regB = field1( state.IDEX.instr );


	/*
		*	Pipeline registers are checked for data hazards
		*	in order of increasing recency in order to forward
		*	the most up-to-date values to the IDEX register.
	*/

	// Check WBEND for data hazards.
	switch( opcode(state.WBEND.instr) )
	{
		/*****************************************/
		case ADD:
		case NAND:
		case CMOV:
		/*
			* These instructions are simple because the updated values
			* are in the writeData register and their target is specified
			* by the destReg of the R-type instr. at this stage.
		*/
			if( regA == field2(state.WBEND.instr) ) {
				newStatePtr->IDEX.readRegA = state.WBEND.writeData;
			}
			if( regB == field2(state.WBEND.instr) ) {
				newStatePtr->IDEX.readRegB = state.WBEND.writeData;
			}
			break;
		/*****************************************/
		case LW: 
		/*
			* In the WB/END pipeline register, the value specified by
			* field1 has already been loaded, and we must simply forward
			* this value to the IDEX pipeline register.
		*/
			if( regA == field1(state.WBEND.instr) ) {
				newStatePtr->IDEX.readRegA = state.WBEND.writeData;
			}
			if( regB == field1(state.WBEND.instr) ) {
				newStatePtr->IDEX.readRegB = state.WBEND.writeData;
			}
			break;
		/*****************************************/
		case BEQ:
		case SW:
		case NOOP:
		case HALT:
			// No action neccessary.
			break;
		/*****************************************/
	}


	// Check MEM/WB for data hazards.
	switch( opcode(state.MEMWB.instr) )
	{
		/*****************************************/
		case ADD:
		case NAND:
		case CMOV:
		/*
			* These instructions are simple because the updated values
			* are in the writeData register and their target is specified
			* by the destReg of the R-type instr. at this stage.
		*/
			if( regA == field2(state.MEMWB.instr) ) 
			{
				newStatePtr->IDEX.readRegA = state.MEMWB.writeData;
			}
			if( regB == field2(state.MEMWB.instr) ) 
			{
				newStatePtr->IDEX.readRegB = state.MEMWB.writeData;
			}
			break;
		///////////////////////////////////////////
		case LW: 
		/*
			* In the MEM/WB pipeline register, the value specified by
			* field1 has already been loaded, and we must simply forward
			* this value to the IDEX pipeline register.
		*/
			if( regA == field1(state.MEMWB.instr) ) {
				newStatePtr->IDEX.readRegA = state.MEMWB.writeData;
			}
			if( regB == field1(state.MEMWB.instr) ) {
				newStatePtr->IDEX.readRegB = state.MEMWB.writeData;
			}
			break;
		/*****************************************/
		case BEQ:
		case SW:
		case NOOP:
		case HALT:

			// No action neccessary.
			break;
		/*****************************************/
	}


	// Check EX/MEM for data hazards.
	switch( opcode(state.EXMEM.instr) )
	{
		/*****************************************/
		case ADD:
		case NAND:
		case CMOV:
		/*
			* Compare the input regA and regB to the destReg of the instr.
			* in EXMEM since it's (in this case) an R-Type and its value will 
			* be waiting in aluResult. This includes CMOV since this will
			* actually be a NOOP if CMOV does not modify its destReg.
		*/
			if( regA == field2(state.EXMEM.instr) ) {
				newStatePtr->IDEX.readRegA = state.EXMEM.aluResult;
			}
			if( regB == field2(state.EXMEM.instr) ) {
				newStatePtr->IDEX.readRegB = state.EXMEM.aluResult;
			}
			break;
		/*****************************************/
		case LW:
		/*
			* If a LW is dected in the EXMEM pipeline, then the CPU has only
			* just calculated the offset for this instr and has not yet been
			* loaded from memory. In this case the only resolution is to stall.
		*/
			if( regA == field1(state.EXMEM.instr)
				|| regB == field1(state.EXMEM.instr) )
			{
				fprintf(stderr, "ERROR: LW not stalled.\n");
				newStatePtr->pc--;
				newStatePtr->IFID = state.IFID;
				newStatePtr->IDEX.instr = NOOPINSTRUCTION;
				//exit(1);
			}
			break;
		/*****************************************/
		case SW:
		case BEQ:
		case NOOP:
		case HALT:
			// No action neccessary.
			break;
		/*****************************************/
	}
}