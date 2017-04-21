/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#ifndef parserDef
#define parserDef
#include "lexer.h"
#include "lexerDef.h"

#define RULECNT		96
#define NTSIZE 		52
#define TSIZE		60

//List of Definitions for Parser

int parse_status;
int no_of_parse_tree_nodes;

typedef enum 
{
program, moduleDeclarations, moduleDeclaration, otherModules, driverModule, module, ret, 
input_plist, N1, output_plist, N2, dataType, type, moduleDef, statements, statement, 
ioStmt, var, whichID, simpleStmt, assignmentStmt, whichStmt, lvalueIDStmt, lvalueARRStmt, 
index_nt, moduleReuseStmt, optional, idList, N3, expression, arithmeticOrBooleanExpr, N7, 
AnyTerm, N8, arithmeticExpr, N4, term_1, N5, factor, op1, op2, logicalOp, relationalOp, 
declareStmt, conditionalStmt, caseStmts, N9, value, default_1, iterativeStmt, range
}non_term;

typedef union
{
	term t;
	non_term nt;
}gElement;

typedef enum {terminal, non_terminal} tag;

term first[TSIZE][20];
term follow[TSIZE][20];

struct gNode
{
	tag t;
	gElement ge;
	struct gNode* next;
};

struct gHead
{
	non_term t;
	struct gNode* top;
};

struct PTStack
{
	struct gNode* top;
	int size;
};

typedef struct pNode *pNode;
typedef struct Queue Queue;

struct Queue
{
	pNode front;
	pNode back;
	int size;
};

struct pNode
{
	tag t;
	gElement e;
	int level, no_of_child;
	tokenInfo* token_link;
	pNode next;
	pNode parent;
	Queue child;
};

struct parseTree
{
	pNode root;
	pNode c_node;
	int height;
};

typedef struct gHead gHead;
typedef struct gNode *gNode;
typedef struct gHead *Grammar;
typedef struct parseTree parseTree;
typedef struct PTStack PTStack;
#endif