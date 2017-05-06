/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#ifndef astDef
#define astDef
#include "parserDef.h"
#include "symbolDef.h"

int no_of_ast_tree_nodes;
typedef struct ASTNode *ASTNode;

struct ASTQueue 
{
	ASTNode front, back;
	int size;
};

struct ASTNode
{
	tag t;
	gElement e;
	int scope;
	term datatype;
	IDEntry* link;
	tokenInfo* token;
	ASTNode next, parent;
	struct ASTQueue childQ;
};

struct AST_Tree
{
	ASTNode root, currentNode;
};

typedef struct ASTQueue ASTQueue;
typedef struct AST_Tree ASTree;
#endif
