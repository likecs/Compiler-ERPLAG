// ASTDef.h
#ifndef astDef
#define astDef
#include "parserDef.h"
#include "symbolDef.h"

int no_of_ast_tree_nodes;

typedef struct ASTNode * ASTNode;
typedef struct ASTQueue ASTQueue;
typedef struct AST_Tree ASTree;

typedef union
{
	int num;
	float rnum;
}valASTNode;

struct ASTQueue 
{
	ASTNode front, back;
	int size;
};

struct ASTNode
{
	ASTNode next, parent;
	ASTQueue childQ;
	gElement e;
	tag t;
	tokenInfo* tokenLink;
	IDEntry* link; //Symbol Table Index
	valASTNode val;
	term datatype;
	int scope;
};

struct AST_Tree
{
	ASTNode root, currentNode;
};
#endif
