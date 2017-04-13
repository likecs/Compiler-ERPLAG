/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */
 
#ifndef parser
#define parser
#include "parserDef.h"
#include <stdio.h>
#include <stdlib.h>

#endif

//List of functions for Initialising Grammer
char *getLexeme(term t);
char *getNonTerm(non_term t);
char *getToken(term t);
void initParseTable();
void fillFirstRules(gNode n, int idx, Grammar G);
void fillFollowRules(non_term nt, int idx, Grammar G);
void createParseTable(Grammar G);
gNode initGrammarNode(gElement ge, tag t);
Grammar createGrammar();
void printGrammar(Grammar G);
void addFirst(term q, int ind);
void addFollow(term q, int ind);
void dfs(non_term node, Grammar G);
void findFirst(Grammar G);
void findFollow(Grammar G);

//List of functions for parser & ParseTree
pNode returnCurrent(pNode p);
Queue initQueue();
Queue enqueue(pNode q, pNode p);
PTStack pop(PTStack s);
PTStack mergeStack(gHead T, PTStack s);
pNode createNonTerminal(pNode Parent, non_term nt, tag t, int no_of_child, int level);
pNode createTerminal(pNode Parent, term term_1, tag t, int level, tokenInfo* token);
parseTree initParseTree();
parseTree parseInputSourceCode(char * testCaseFile, hashTable h, Grammar G, int print);
void printParseTreeQueue(FILE *fp, pNode p);
void printParseTree(char *outfile, char * testCaseFile, hashTable h, Grammar G);
