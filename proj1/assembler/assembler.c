#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*****************************************************************************/
#define MAXLINELENGTH 1000	// Max length of a single line of code.
#define MAXLINES 50			// Max assembly program length.
#define MODE_READ "rt" 		// Pneumonic for read mode.
#define MODE_WRITE "w+"		// Pneumonic for write mode.
/*****************************************************************************/
// Symbolic Link Linked List
typedef struct SymbolNode 
{
	char label[MAXLINELENGTH];
	int address;
	struct SymbolNode *next;
} SymbolicAddressNode;
// Instruction Container.
typedef struct InstructionNode
{
	int address;

	char opcode[MAXLINELENGTH];
	char arg0[MAXLINELENGTH];
	char arg1[MAXLINELENGTH];
	char arg2[MAXLINELENGTH];

} InstructionNode;
/*****************************************************************************/
// DECLARATIONS
int readAssembly( FILE*, SymbolicAddressNode*, InstructionNode[] );
void writeBinary( FILE*, SymbolicAddressNode*, InstructionNode[], int );

int parse( SymbolicAddressNode*, InstructionNode );
int r_type( int, char*, char*, char* );
int i_type( int, char*, char*, int );
inline void prepareFile( FILE**, const char*, char* );
inline int isValidLabel( char* );
inline void validLineLength( char*, int );

void addSymbol( SymbolicAddressNode*, char*, int );
int getSymbolAddress( SymbolicAddressNode*, char* );


void printSymbols(SymbolicAddressNode *root)
{
	SymbolicAddressNode *node = root->next;

	while( node != 0 )
	{
		printf( "Symbol: %s (address: %i)\n", 
				(*node).label, 
				(*node).address );

		node = node->next;
	}
}

/******************************************************************************
 	 * Entry Point
******************************************************************************/
int main( int argc, const char* argv[] )
{
	int debug = 0;
	// Verify requisite files.
	if( argc == 4 )
	{
		if(strcmp(argv[3], "-d") == 0)
		{
			debug = 1;
		}
	}
	else if( argc != 3 ) {
		fprintf( stderr, 
				 "ERROR: Usage: %s <assembly-code-file> <machine-code-file>\n",
				 argv[0] );
		exit(1);
	}

	// Prepare files for reading.
	FILE* infile;
	FILE* outfile;
	prepareFile( &infile, argv[1], MODE_READ );
	prepareFile( &outfile, argv[2], MODE_WRITE );

	// Prepare for assembling.
	SymbolicAddressNode *symbol_list;
	symbol_list = malloc( sizeof(SymbolicAddressNode) );
	symbol_list->next = 0;

	InstructionNode instructions[MAXLINES];

	// First Pass. Parse incoming file; prepare for conversion.
	int instruction_counter = readAssembly( infile, symbol_list, instructions );

	///////////////////////////////////////////////
	// for verification:
	if( debug == 1 )
	{
		int i;
		printf("Addresses: \n");
		for( i = 0; i < instruction_counter; i++ )
		{
			printf( "%i: %s %s %s %s\n", 
					i,
					instructions[i].opcode, 
					instructions[i].arg0, 
					instructions[i].arg1, 
					instructions[i].arg2 );
		}
		printf("Symbols: \n");
		printSymbols( symbol_list );
	}
	///////////////////////////////////////////////

	writeBinary( outfile, symbol_list, instructions, instruction_counter );

	exit(0);
}

/******************************************************************************
	 * FIRST PASS: READ
	 *	- Parse input into an array.
	 *		- Array indexed by instruction address.
	 *	- Parse symbolic addresses.
	 *	- Fail on Error.
	 *
	 * @param infile		The file to parse.
	 * @param symbolsRoot	The head of the Symbols LL.
	 * @param addresses[]	An array to put each line of code.
	 *
	 * @return		The number of addresses contained in the program.
******************************************************************************/
int readAssembly( FILE* infile, 
				  SymbolicAddressNode* root, 
				  InstructionNode instructions[] )
{
	int i = 0;		// indexor.
	
	// temporary storage.
	char *itr;
	char line[MAXLINELENGTH];
	char label[MAXLINELENGTH];

	while( fgets(line, MAXLINELENGTH, infile) != NULL )
	{
		itr = line;
		instructions[i].address = i;

		// Check if a symbol is to be added.
		if( sscanf(line, "%[^\t\n ]", label) )
		{
			addSymbol( root, label, i );

			// Skip over label.
			itr += strlen( label );
		}

		// Pick up the rest.
		sscanf( itr, 
				"%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]",
				instructions[i].opcode, 
				instructions[i].arg0, 
				instructions[i].arg1, 
				instructions[i].arg2 );

		i++;

		// Ensure that maximum LoC length is obeyed.
		if( i > MAXLINES )
		{
			fprintf( stderr, 
					 "ERROR: Maximum program length exceeded. Maximum: %i\n", 
					 MAXLINES );
			exit(1);
		}
	}

	return i;
}

/******************************************************************************
	 * SECOND PASS
	 *	Parse instruction list, convert each InstructionNode to its binary
	 *	representation.
	 *
	 * @param outfile			the file to write the binary representation to.
	 * @param root 				the root node of the SymbolicAddress linked list.
	 * @param instructions		the array of instructions.
	 * @param counter			the size of the instructions array.
******************************************************************************/
void writeBinary( FILE* outfile, 
				  SymbolicAddressNode* root, 
				  InstructionNode instructions[],
				  int counter )
{
	int i;

	for( i = 0; i < counter; i++ )
	{
		fprintf( outfile, "%i\n", parse( root, instructions[i] ) );
	}
}

/******************************************************************************
	 * Parse a line of assembly.
	 *
	 * @param root  	SymbolicAddressNode root pointer.
	 * @param inode		a single InstructionNode representing a line of code.
******************************************************************************/
int parse( SymbolicAddressNode* root, InstructionNode inode )
{
	if( strcmp( "add", inode.opcode ) == 0)
	{
		// opcode: 0
		return r_type((0 << 22), inode.arg0, inode.arg1, inode.arg2 );
	}
	else if (strcmp( "nand", inode.opcode ) == 0)
	{
		// opcode: 1
		return r_type( (1 << 22), inode.arg0, inode.arg1, inode.arg2 );
	}
	else if (strcmp( "lw", inode.opcode ) == 0)
	{
		// opcode: 2
		if( isalpha(inode.arg2[0]) != 0 
			&& isValidLabel(inode.arg2) != 1 )
		{
			fprintf( stderr, 
					 "ERROR: %s is not a valid label. (see address %i)\n", 
					 inode.arg2,
					 inode.address );
			exit(1);
		}
		
		int offset = (isValidLabel(inode.arg2))
				   ? (getSymbolAddress(root, inode.arg2))
				   : checkBits(atoi(inode.arg2), 16);

		return i_type( (2 << 22), inode.arg0, inode.arg1, offset );
	}
	else if (strcmp( "sw", inode.opcode ) == 0)
	{
		// opcode: 3
		if( isalpha(inode.arg2[0]) != 0 
			&& isValidLabel(inode.arg2) != 1 )
		{
			fprintf( stderr, 
					 "ERROR: %s is not a valid label. (see address %i)\n", 
					 inode.arg2,
					 inode.address );
			exit(1);
		}
		int offset = (isValidLabel(inode.arg2))
				   ? (getSymbolAddress(root, inode.arg2))
				   : checkBits(atoi(inode.arg2), 16);

		return i_type( (3 << 22), inode.arg0, inode.arg1, offset );
	}
	else if (strcmp( "beq", inode.opcode ) == 0)
	{
		// opcode: 4
		if( isalpha(inode.arg2[0]) != 0 
			&& isValidLabel(inode.arg2) != 1 )
		{
			fprintf( stderr, 
					 "ERROR: %s is not a valid label. (see address %i)\n", 
					 inode.arg2,
					 inode.address );
			exit(1);
		}
		// check symbols. calculate offset relative to the
		// address of this instruction before passing to 
		// the converter.
		int offset = (isValidLabel(inode.arg2))
				   ? ( getSymbolAddress(root, inode.arg2) - inode.address )
				   : checkBits( atoi(inode.arg2), 16 );

		// hacky fix
		if( isValidLabel(inode.arg2) 
			&& getSymbolAddress(root, inode.arg2) > inode.address )
		{
			--offset;
		}

		return i_type( (4 << 22), inode.arg0, inode.arg1, offset );
	}
	else if (strcmp( "cmov", inode.opcode ) == 0)
	{
		// opcode: 5
		// r_type.
		return r_type( (5 << 22), inode.arg0, inode.arg1, inode.arg2 );
	}
	else if (strcmp( "halt", inode.opcode ) == 0)
	{
		// o-type instructions are easy.
		return ( 6 << 22 );
	}
	else if (strcmp( "noop", inode.opcode ) == 0)
	{
		// o-type instructions are easy.
		return ( 7 << 22 );
	}
	else if (strcmp( ".fill", inode.opcode ) == 0)
	{
		if (isValidLabel( inode.arg0 ))
		{
			fprintf(stderr, "I am going to die: %s\n", inode.arg0);
			return getSymbolAddress( root,  inode.arg0 );
		}
		else
		{
			return atoi(inode.arg0);
		}
	}
	else
	{
		fprintf( stderr, 
				 "ERROR: Unrecognized opcode %s at address %i\n", 
				 inode.opcode,
				 inode.address );
	}
}

/******************************************************************************
	 * Convert arguments to a bitstring representing an r_type instruction.
	 *
	 * @param op 		the bitshifted opcode.
	 * @param reg1		a string representation of field0 (regA)
	 * @param reg2 		a string representation of field1 (regB)
	 * @param dest 		a string representation of field1 (destReg)
******************************************************************************/
int r_type( int op, char* reg1, char* reg2, char* dest )
{
	int regA = checkBits( atoi(reg1), 3);
	int regB = checkBits( atoi(reg2), 3);
	int destReg = checkBits( atoi(dest), 3);

	return (op | (regA << 19) | (regB << 16) | destReg);

}

/******************************************************************************
	 * Convert arguments to a bitstring representing an i_type instruction.
	 * This function takes care of converting a negative offset to a 16-bit
	 * two's compliment number.
	 *
	 * @param op 		the bitshifted opcode.
	 * @param reg1		a string representation of field0 (regA)
	 * @param reg2 		a string representation of field1 (regB)
	 * @param offset 	a calculated value for the offset.
******************************************************************************/
int i_type( int op, char* reg1, char* reg2, int offset )
{
	int regA = checkBits( atoi(reg1), 3);
	int regB = checkBits( atoi(reg2), 3);
	checkBits( offset, 16 );

	if( offset < 0 )
	{
		offset = 0xFFFF + offset;
	}

	return (op | (regA << 19) | (regB << 16) | offset);
}

/******************************************************************************
	 * Verify that a number will fit in a certain number of bits.
	 *
	 * @param bits		the number to check.
	 * @param max		what the maximum should be. (expecting 3 or 16)
******************************************************************************/
int checkBits( int bits, int max)
{
	if ( max == 3  && bits > 7 )
	{
		fprintf( stderr, 
				 "ERROR: Value %d does not fit in 3-bits (expected register value.)\n", 
				 bits );
		exit(1);
	}
	else if ( max == 16 && (bits < -32768 ||  bits > 32767))
	{
		fprintf( stderr, 
				 "ERROR: Value %d does not fit in 16-bits (expected offset field)\n", 
				 bits );
		exit(1);
	}
	else
	{
		return bits;
	}
}

/******************************************************************************
	 * Prepare a file for IO operations.
	 *
	 * @param FILE 		the file pointer to use.
	 * @param name		the name of the file to be opened
	 * @param mode		the mode to use; (in fopen).
******************************************************************************/
inline void prepareFile( FILE **file, 
						 const char *name, 
						 char *mode ) 
{
	*file = fopen(name, mode);
	
	if (file == NULL) 
	{
		fprintf( stderr,
				 "ERROR: Can't open file: %s!\n",
				 name );
		// Terminate. (Fatal Error)
		exit(1);
	}
}

/******************************************************************************
	 * Ensures label satifies requirements
	 * 
	 * @param label[]	the string to check.
	 * 
	 * @return	0 if label is invalid; 1 otherwise.
******************************************************************************/
inline int isValidLabel( char* label )
{
	// Label length must be at most 6.
	if ( strlen(label) > 6 ) 
	{
		return 0;
	}
	else if ( isalpha(label[0]) == 0 )
	{
		return 0;
	}
	else
	{
		int i;
		for ( i = 0; i < strlen(label); i++ )
		{
			if ( isalnum(label[0]) == 0 )
			{
				return 0;
			}
		}
	}

	return 1;
}

/******************************************************************************
 	 * Ensure the line length limit is obeyed.
 	 *
 	 * @param line 	the line to validate.
******************************************************************************/
inline void validLineLength( char* line, int i )
{
	// Ensure line length is obeyed.
	if( strchr(line, '\n') == NULL
		&& strchr(line, '\0') == NULL )
	{
		fprintf( stderr, 
				 "ERROR: Line %d is too long.\n", 
				 i);
		exit(1);
	}
}

/******************************************************************************
	 * Add a new SymbolicAddressNode to the currently maintained linked list.
	 * 
	 * @param root		root SymbolicAddressNode
	 * @param label 	the label of the symbolic address.
	 * @param address 	the address referred to by label.
******************************************************************************/
void addSymbol( SymbolicAddressNode* root, char* label, int address )
{
	if( isValidLabel(label) == 0 )
	{
		fprintf( stderr, 
				 "ERROR: %s is not a valid label. (see address %d)\n", 
				 label,
				 address );
		exit(1);
	}

	SymbolicAddressNode* node = root;

	while( node->next != 0 )
	{
		node = node->next;
		
		if( strcmp( node->label, label ) == 0 )
		{
			fprintf(stderr, 
					 "ERROR: Duplicate label, %s, found. (see address %d)\n", 
					 label,
					 address );
			exit(1);
		}
	}

	node->next = malloc( sizeof(SymbolicAddressNode) );
	node = node->next;
	strcpy( node->label, label );
	node->address = address;
}

/******************************************************************************
	 * Find the address of a symbolic address by name.
	 * 
	 * @param root		root SymbolicAddressNode
	 * @param label 	the label of the symbolic address.
	 *
	 * @return 			the address referred to by the label.
******************************************************************************/
int getSymbolAddress( SymbolicAddressNode* root, char* label )
{
	if( isValidLabel(label) == 0 )
	{
		fprintf( stderr, 
				 "ERROR: %s is not a valid label.\n", 
				 label );
		exit(1);
	}

	SymbolicAddressNode* node = root;

	while( node->next != 0 )
	{
		node = node->next;

		if( strcmp( node->label, label ) == 0 )
		{
			return node->address;
		}
	}

	fprintf( stderr, 
			 "ERROR: Undeclared label %s.\n", 
			 label );
	exit(1);
}