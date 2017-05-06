/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#ifndef assembler
#define assembler
#include "ast.h"

void WriteData(hashTable2 h_ID, FILE * fp );
void writeNode(ASTNode a,FILE * fp);
void parseQueue(ASTNode a,FILE * fp);


#endif