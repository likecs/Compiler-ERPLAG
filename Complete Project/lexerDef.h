/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */
 
#ifndef lexerDef
#define lexerDef

//List of Definitions for Hashing Keywords

typedef enum{available, occupied, deleted}status;

#define hash_capacity 31

struct linklist 
{
	int code;
	char word[21];
	struct linklist *next;
};

struct store 
{
	int code;
	char word[21];
};

struct entry 
{
	int code;
	char word[21];
	status flag;
	struct linklist *start;
};

typedef struct entry entry;
typedef struct store store;
typedef struct linklist linklist;
typedef entry *hashTable;

//List of Definitions for Lexemes/Toxens

typedef enum 
{
INTEGER, REAL, BOOLEAN, OF, ARRAY, START, END, DECLARE, MODULE, DRIVER,
PROGRAM, GET_VALUE, PRINT, USE, WITH, PARAMETERS, TRUE_, FALSE_, TAKES,
INPUT, RETURNS, AND, OR, FOR, IN, SWITCH, CASE, BREAK, DEFAULT, WHILE, 
PLUS, MINUS, MUL, DIV, LT, LE, GE, GT, EQ, NE, DRIVERDEF, DRIVERENDDEF,
DEF, ENDDEF, COLON, RANGEOP, SEMICOL, COMMA, ASSIGNOP, SQBO, SQBC, BO, 
BC, COMMENTMARK, ID, NUM, RNUM, $, e, null_point = -1
}term;

struct tokenInfo 
{
	term s;
	//common place to store the identifiers
	union
	{
		int i;			//constant integer value
		float f;		//constant real value
		char s[21];		//function names, id names
	}val;
	//lno = line number, df = cnt for constant int/real number
	int lno, df;		
	struct tokenInfo *n;
	struct tokenInfo *prev;
};//term structure

typedef struct tokenInfo tokenInfo;
#endif
