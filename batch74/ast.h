#ifndef ast 
#define ast

#include "astDef.h"

#endif

ASTNode createASTNode(ASTNode parent, gElement e, tag t, tokenInfo* tokenLink, IDEntry* link, valASTNode val);
ASTree buildASTree(parseTree PT, hashTable2 tableID, hashTable2 tableFunc, int print);
ASTNode FillNodeAST(pNode p, ASTNode parent, hashTable2 tableID, hashTable2 tableFunc);
ASTNode AddToQueueAST(pNode p, ASTNode a, ASTNode parent,hashTable2 tableID, hashTable2 tableFunc);
ASTQueue enqueueAST(ASTQueue q, ASTNode p);
int ASTQueueTypeChecker(ASTNode a, int ToPrint);
ASTNode typeAssignerAndChecker(ASTNode a, int ToPrint);
void printASTree(ASTree a, int print);
void printASTreeQueue(ASTNode a, int print);
void printASTNode(ASTNode a, int print);