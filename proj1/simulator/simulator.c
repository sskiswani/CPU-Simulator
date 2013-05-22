#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
//////////////////////////////
// DEFINITIONS
#define NUMMEMORY 65536 /* max number of words */
#define NUMREGS 8		/* number of registers */
#define MAXLINELENGTH 1000
//////////////////////////////
typedef struct stateStruct {
	int pc;
	int mem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
} stateType;
//////////////////////////////
// DECLARATIONS
void printState( stateType* );
int convertNum( int );

inline void r_getArgs( int, int* );
inline void i_getArgs( int, int* );

//////////////////////////////

int debug;

int main( int argc, const char *argv[] )
{
	debug = 0;
	// Ensure valid arguments.
	if (argc == 3 && strcmp( argv[2], "-d") == 0)
	{
		fprintf( stderr, "***********DEBUG MODE ENABLED***********\n");
		debug = 1;
	}
	else if( argc != 2 )
	{
		fprintf( stderr,
				 "error: usage: %s <machine-code file>\n",
				 argv[0] );
	}

	// Attempt to open the file.
	FILE *infile = fopen( argv[1], "r" );
	if( infile == NULL )
	{
		fprintf( stderr, 
				 "ERROR: Can't open file %s\n", 
				 argv[1] );
		perror("fopen");
		exit(1);
	}

	//////////////////////////
	// Initialize State.
	stateType state;
	char line[MAXLINELENGTH];
	unsigned int i;

	// Initialize registers.
	for( i = 0; i < NUMREGS; i++ ) {
		state.reg[i] = 0;
	}

	/* Read the file. */
	for ( state.numMemory = 0;
		  fgets(line, MAXLINELENGTH, infile) != NULL;
		  state.numMemory++ )
	{
		// Check for illegible lines.
		if ( sscanf(line, "%d", state.mem + state.numMemory) != 1) 
		{
			fprintf( stderr,
					 "ERROR: Failed to read address %d\n", 
					 state.mem[state.numMemory] );
			exit(1);
		}
		else if( state.numMemory >= NUMMEMORY )
		{
			fprintf( stderr, 
					 "ERROR: Max number of words exceeded. Max: %d\n", 
					 NUMMEMORY );
			exit(1);
		}
		else
		{
			// Print initial memory.
			printf( "memory[%d]=%d\n", 
					state.numMemory, 
					state.mem[state.numMemory] );
			
		}
	}
		printf("\n");

	//////////////////////////
	/* Begin Simulation.    */
	state.pc = 0;
	unsigned int total = 0;	// instruction execution counter
	int op_args[3];			// counter of arguments.
	int regA, regB, offset;	// some temporary variables for legibility purposes.

	// using an unsigned int so that if loop exceeds memory, value resets.
	for ( i = 1; i != 0; i++ ) 
	{
		// Before every iteration...
		if(debug == 1) 
		{
			fprintf(stderr, "run: %d\n", i);
		}
		total++;
		printState( &state );

		// The meat.
		switch ( state.mem[state.pc] >> 22 )
		{
			case 0: // ADD
				r_getArgs( state.mem[state.pc], op_args );

				// 0 = regA, 1 = regB, 2 = destReg
				regA = state.reg[ op_args[0] ];
				regB = state.reg[ op_args[1] ];

				// add contents of regA with contents of regB
				// store results in destReg.
				state.reg[ op_args[2] ] = regA + regB;

				break; 

			case 1: // NAND 
				r_getArgs( state.mem[state.pc], op_args );
				
				// 0 = regA, 1 = regB, 2 = destReg
				regA = state.reg[ op_args[0] ];
				regB = state.reg[ op_args[1] ];

				// nand contents of regA with contents of regB,
				// store results in destReg.
				state.reg[ op_args[2] ] = ~(regA & regB);

				break;

			case 2: // LW 
				i_getArgs( state.mem[state.pc], op_args );

				// 0 = regA, 1 = regB, 2 = offsetField
				offset = op_args[2] + state.reg[ op_args[0] ];

				if( offset < 0 || offset >= state.numMemory )
				{
					fprintf( stderr, 
							 "ERROR: Invalid memory address: %d (out of range).\n", 
							 offset);
					exit(1);
				}

				// load regB from memory. Memory address is formed by
				// adding offsetField with the contents of regA.
				state.reg[ op_args[1] ] = state.mem[ offset ];
				
				break;
			////////////////////////////////////////////////////
			case 3: // SW 
				i_getArgs( state.mem[state.pc], op_args );

				// 0 = regA, 1 = regB, 2 = offsetField
				offset = op_args[2] + state.reg[ op_args[0] ];

				if( offset < 0 || offset >= state.numMemory )
				{
					fprintf( stderr, 
							 "ERROR: Invalid memory address: %d (out of range).\n", 
							 offset);
					exit(1);
				}
				// store regB into memory. Memory address is formed by adding
				// offsetField with the contents of regA.
				state.mem[ offset ] = state.reg[ op_args[1] ];

				break;
			////////////////////////////////////////////////////
			case 4: // BEQ
				i_getArgs( state.mem[state.pc], op_args );

				// 0 = regA, 1 = regB, 2 = offsetField
				regA = state.reg[ op_args[0] ];
				regB = state.reg[ op_args[1] ];

				// if the contents of regA and regB are the same, then branch
				// to the address PC+1+offsetField, where PC is the address of the
				// beq instruction.
				if( regA == regB )
				{
					state.pc = state.pc + op_args[2];
					// the +1 is handled by the regular increment.
				}

				break;
			
			case 5: // CMOV
				r_getArgs( state.mem[state.pc], op_args );
				
				// 0 = regA, 1 = regB, 2 = destReg
				regA = state.reg[ op_args[0] ];
				regB = state.reg[ op_args[1] ];

				// copy the value regA into destReg if the contents of regB != 0
				if( regB != 0 )
				{
					state.reg[ op_args[2] ] = regA;
				}

				break;
			
			case 6: // HALT

				// Terminating condition.
				i = -1;

				break;

			case 7: // NOOP

				// do nothing.

				break;
			
			default: // idk?
				fprintf( stderr, 
						 "ERROR: Memory at location %i illegible.\n", 
						 state.pc );
				exit(1);
				break;
		}

		state.pc++;
		
		if( total > 200 && debug == 0 )
		{
					fprintf( stderr, 
							 "ERROR: Simulation exceeding 200 instructions; will be terminated.\n");
					fprintf( stderr, 
							 "ERROR: If you'd like to iterate until unsigned-int overflow, launch using:" );
					fprintf( stderr, "<simulate> <machine-code file> -d\n" );
					i = -1;
		}
		
		// Error-checking.
		if( state.pc > state.numMemory || state.pc < 0 )
		{
			fprintf( stderr, 
					 "ERROR: PC is out of range [0, %d). PC value: %d\n", 
					 state.numMemory,
					 state.pc );
			exit(1);	
		}

	} // end simulation loop.

	printf("machine halted\n");
	printf("total of %d instructions executed\n", total);
	printf("final state of machine:\n");
	printState( &state );

	exit(0);
}

/*****************************************************************************/

/**
	 * Prints the state of the simulator.
	 *
	 * @param statePtr	summary of the state of the machine.
 */
void printState( stateType *statePtr )
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
	for (i = 0; i < statePtr->numMemory; i++) 
	{
	    printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
	}
    printf("\tregisters:\n");
	for (i = 0; i < NUMREGS; i++) 
	{
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}
    printf("end state\n");
}

/*****************************************************************************/

/**
	 * Decode an R-type instruction, putting its arguments in a container.
	 * This method will do some rudimentary error checking.
	 *
	 * @param bitstring		the bitstring representing the instruction.
	 * @param container	 	container for instruction arguments.
*/
inline void r_getArgs(int bitstring, int* arg)
{
	arg[0] = ( (bitstring & (7<<19)) >> 19 );
	arg[1] = ( (bitstring & (7<<16)) >> 16 );
	arg[2] = ( bitstring & 7 );

	if( arg[0] < 0 || arg[1] < 0 || arg[2] < 0 )
	{
		fprintf( stderr, "ERROR: Negative register value encountered.\n");
		exit(1);
	}
	else if ( arg[0] >= NUMREGS || arg[1] >= NUMREGS || arg[2] >= NUMREGS )
	{
		fprintf( stderr, 
				 "ERROR: Register value greater than %d encountered.\n",
				 NUMREGS );
		exit(1);
	}
}


/**
	 * Decode an I-type instruction, putting its arguments in a container.
	 *
	 * @param bitstring		the bitstring representing the instruction.
	 * @param container	 	container for instruction arguments.
*/
inline void i_getArgs(int bitstring, int* arg)
{
	arg[0] = ( (bitstring & (7<<19)) >> 19 );
	arg[1] = ( (bitstring & (7<<16)) >> 16 );
	arg[2] = ( bitstring & (0xFFFF) );

	if( arg[2] & (1<<15) )
	{
		arg[2] -= (1<<16);
	}

	if( arg[0] < 0 || arg[1] < 0 )
	{
		fprintf( stderr, "ERROR: Negative register value encountered.\n");
		exit(1);
	}
	else if ( arg[0] >= NUMREGS || arg[1] >= NUMREGS )
	{
		fprintf( stderr, 
				 "ERROR: Register value greater than %d encountered.\n",
				 NUMREGS );
		exit(1);
	}
}