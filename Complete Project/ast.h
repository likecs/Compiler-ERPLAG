/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#ifndef ast 
#define ast

#include "astDef.h"

#endif

int isValidTerminal(term t);
int isLogicalOperator(term op);
int isArithmeticOperator(term op);
int isRelationalOperator(term op);

//functions for creation of abstract syntax tree
ASTQueue enqueueAST(ASTQueue q, ASTNode p);
ASTree buildASTree(parseTree PT, hashTable2 tableID, hashTable2 tableFunc, int print);
ASTNode createASTNode(ASTNode parent, gElement e, tag t, tokenInfo* tokenLink, IDEntry* link);
ASTNode fillASTNode(pNode p, ASTNode parent, hashTable2 tableID, hashTable2 tableFunc);
ASTNode fillASTQueue(pNode p, ASTNode a, ASTNode parent,hashTable2 tableID, hashTable2 tableFunc);

//functions for checking the semantic errors in expressions, if they exist
int ASTQueueTypeChecker(ASTNode a, int print);
ASTNode typeAssignerAndChecker(ASTNode a, int print);

//functions for printing abstract syntax tree
void printASTree(ASTree a, int print);
void printASTreeQueue(ASTNode a, int print);
void printASTNode(ASTNode a, int print);