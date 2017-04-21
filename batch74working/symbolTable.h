/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#ifndef symbolTable
#define symbolTable
#include "symbolDef.h"

//List of functions for Hashing Scoped variables/functions

hashTable2 initializeHashTable2(int l);
int hashValue2(char *key);
store2 findScope(hashTable2 T, char *searchKey, int scope, int option);
void addScope(hashTable2 T, IDEntry* M, int option);

//List of functions for Symbol table

void createIDTable(tokenInfo* token, totalScopeList* scope_tot, hashTable2 tableID, hashTable2 tableFunc, int print);
void secondRun(tokenInfo* token, hashTable2 tableID, hashTable2 tableFunc, int print);
int checkModuleReturn(IDEntry* ctrl_var, tokenInfo* token, hashTable2 tableID);
void checkFor(IDEntry* ctrl_var, tokenInfo* token, hashTable2 tableID, int print);
void printVariables(hashTable2 table, totalScopeList *par);
void printFunction(hashTable2 table, totalScopeList* par);
int getBytes(term s);
#endif