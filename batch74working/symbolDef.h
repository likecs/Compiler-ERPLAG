/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#ifndef symbolDef
#define symbolDef

#include "lexerDef.h"

//List of Definitions for Hashing Scoped variables/functions

typedef struct IDEntry IDEntry;

typedef enum{defined, declared}funcStatus;

#define hash_capacity_2 31

struct linklist2 
{
	IDEntry *ID;
	struct linklist2 *next;
};

struct store2 
{
	int code;
	IDEntry *node;
};

struct entry2 
{
	IDEntry *ID;
	status flag;
	struct linklist2 *start;
};

typedef struct store2 store2;
typedef struct entry2 entry2;
typedef struct linklist2 linklist2;
typedef entry2 *hashTable2;

//List of Definitions for Symbol Table entries for variables/functions

int semantic_status;

struct variable
{
	term type;
};

struct array 
{
	term type;
	int s_idx, e_idx;
};

struct IDVariable 
{
	char word[21];
	int lno;
	int scope;
	int code;
	int v_a;
	int bytes;
	int depth;
	int offset;
	char func_name[21];
	union
	{
		struct variable v;
		struct array a;
	}var;
};

struct parameter
{
	union
	{
		struct variable v;
		struct array a;
	}var;
	int v_a;
	struct parameter *next;
};

struct IDFunction 
{
	char word[21];
	int lno;
	int used;
	int scope;
	funcStatus status;
	struct parameter *inputList;
	struct parameter *outputList;
};

struct IDEntry 
{
	union
	{
		struct IDVariable ivar;
		struct IDFunction ifunc;
	}entity;
};

struct scopeChain 
{
	int scope;
	int in_cond;
	int in_loop;
	struct scopeChain *next;
	struct scopeChain *prev;
};

struct totalScopeList
{
	int scope_start;
	int scope_end;
};

typedef struct variable variable;
typedef struct array array;
typedef struct IDVariable IDVariable;
typedef struct scopeChain scopeChain;
typedef struct totalScopeList totalScopeList;
typedef struct parameter parameter;
typedef struct IDFunction IDFunction;
#endif
