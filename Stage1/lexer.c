/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#include "lexerDef.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int id_num = 1;
int error_exist = 0;
int line_num = 1;
int comment_flag = 0;
tokenInfo* end;
tokenInfo* first;
tokenInfo* scan;
FILE *f_write;
static FILE* f_read;
int BUFFSIZE = 256;

int isAlpha(char c) 
{
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
	{
		return 1;
	}
	return 0;
}

int isDigit(char c) 
{
	if (c >= '0' && c <= '9')
	{
		return 1;
	}
	return 0;
}

int expo(int e, int n) 
{
	int res = 1, val = e;
	while(n) 
	{
		if (n & 1)
		{
			res = res * val;
		}
		val = val * val;
		n /= 2;
	}
	return res;
}

char *getStream()
{
	//Get line from input file into bufferfer
	char *temp = (char *)malloc(BUFFSIZE * sizeof(char));
	fgets(temp, BUFFSIZE, f_read);
	if (feof(f_read))
	{
		//reached end of file
		return "";
	}
	if (strlen(temp) == BUFFSIZE)
	{
		int pos = (BUFFSIZE - 1);
		//remove spaces and tabs
		while((pos >= 0) && (!(temp[pos]==' ' || temp[pos]=='\t')))
		{
			pos -= 1;
		}
		temp[pos + 1] = '\0';
		fseek(f_read, -((BUFFSIZE - 1) - pos), SEEK_CUR);
		if (pos < 0)
		{
			//when no spaces are there in buffer (in order to avoid infinite loop)
			fgets(temp, BUFFSIZE, f_read);
		}
	}
	return temp;
}

tokenInfo* getFirstToken()
{
	return first->n;
}

tokenInfo* getNextToken()
{
	if(scan == NULL)
	{
		scan = first;
	}
	else
	{
		scan = scan->n;
	}
	return scan;
}

tokenInfo* initToken()
{
	//Initialise the basic common values of Token
	tokenInfo* temp = (tokenInfo *)malloc(sizeof(tokenInfo));
	temp->lno = line_num;
	temp->n = NULL;
	temp->prev = end;
	end->n = temp;
	end = end->n;
	return temp;
}

void addDollar()
{
	//Add Dollar to end of file
	tokenInfo *temp = initToken();
	temp->s = $;
}

void addSym(int n)
{
	//Add a symbol to the tokenstream
	tokenInfo* temp = initToken();
	int i, done = 0; 
	for(i = INTEGER; i != ID; ++i)
	{
		if (i == n)
		{
			done = 1;
			temp->s = (term)i;
			break;
		}

	}
	if (done == 0)
	{
		FILE *f_write;
		f_write = fopen("error.txt","a");
		printf("Wrong symbol number %d passed to addSym\n",n);
		fclose(f_write);
	}
}

void addID(char *id)
{
	//Add an ID to the tokenstream
	tokenInfo* temp = initToken();
	temp->s = ID;
	strcpy(temp->val.s, id);
}	

void addRNum(float num, int rnum_no)
{
	//Add a floating point number to tokenstream
	tokenInfo* temp = initToken();
	temp->s = RNUM;
	temp->val.f = num;
	temp->df = rnum_no;
}
	
void addNum(int num, int num_no)
{
	//Add an integer to tokenstream
	tokenInfo* temp = initToken();
	temp->s = NUM;	
	temp->val.i = num;
	temp->df = num_no;
}

void DFA(hashTable table, char *buffer, int pr)
{
	//DFA for pattern matching and assigning correct token, if possible
	int pos = 0;
	int l = strlen(buffer);
	char lookahead = buffer[0];
	static int num_no = 1;
	static int rnum_no = 1;
	while(pos < l)
	{
		//ignore content between the comments
		if (comment_flag == 1)
		{
			while((pos < l) && (!(buffer[pos] == '*' && buffer[pos+1] == '*')))
			{
				if (buffer[pos] == '\n')
				{
					line_num += 1;
				}
				pos += 1;
			}
			if (pos < l && buffer[pos] == '\n')
			{
				line_num += 1;
			}
			if(pos == l)
			{
				pos -= 1;
			}
			lookahead = buffer[pos];
		}

		if (isAlpha(lookahead))
		{
			//ID or keyword is checked through hashing
			char id[21] = "";
			int i = 0;
			id[i] = lookahead;
			pos += 1;
			while ((pos < l) && ((isAlpha(buffer[pos]) || buffer[pos] == '_' || isDigit(buffer[pos])) && buffer[pos] != '\n'))
			{
				id[++i] = buffer[pos++];
			}
			store res = findKeyword(table, id);
			if (res.code == -1)
			{
				//ID as not found in hash table
				if (strlen(id) <= 8)
				{
					if (pr == 1)
					{
						printf("Line Number : %d, Lexeme : ID - %s\n", line_num, id);
					}
					addID(id);
				}
				else
				{
					if(pr == 1)
					{
						printf("ERROR : Identifier %s at line %d is longer than 8 characters.\n", id, line_num);
					}
					error_exist = 1;
				}
			}
			else
			{
				//Keyword as found in hash table
				if (pr == 1)
				{
					int j, len = strlen(res.word);
					printf("Line Number : %d, Lexeme : KEYWORD - ", line_num);
					for(j = 0; j < len; ++j)
					{
						printf("%c", toupper(res.word[j]));
					}
					printf("\n");
				}
				addSym(res.code);
			}
			pos -= 1;
		}
		else if (isDigit(lookahead))
		{
			//NUM or RNUM
			int num1 = 0, num2 = 0;
			int dec = 0;
			int power = 0;			
			num1 = buffer[pos] - '0';
			while((pos+1 < l) && isDigit(buffer[pos+1]))
			{
				num1 = (num1 * 10) + (buffer[pos+1]-'0');
				pos += 1;
			}
			if((pos+1 < l) && buffer[pos+1] == '.')
			{
				if((pos+2 < l) && buffer[pos+2] != '.')
				{	
					pos += 1;
					if((pos+1 < l) && (!isDigit(buffer[pos+1])))
					{
						if(pr == 1)
						{
							printf("ERROR : Invalid real number at line %d, column %d : No digit after floating point.\n", line_num, (pos+1));
						}
						error_exist = 1;
						pos += 1;
					}
					while((pos+1 < l) && isDigit(buffer[pos+1]))
					{
						pos += 1;						
						num2 = (num2 * 10) + (buffer[pos] - '0');
						dec++;
					}
					if((pos+1 < l) && (buffer[pos+1] == 'e' || buffer[pos+1] == 'E'))
					{
						pos += 2;
						if(buffer[pos] == '+')
						{
							while((pos+1 < l) && isDigit(buffer[pos+1]))
							{
								pos += 1;
								power = (power * 10) + (buffer[pos] - '0');
							}
						}
						else if(buffer[pos] == '-')
						{
							while((pos+1 < l) && isDigit(buffer[pos+1]))
							{
								pos += 1;
								power = (power * 10) + (buffer[pos] - '0');
							}
							power = -(power);
						}
						else if(isDigit(buffer[pos]))
						{
							while(isDigit(buffer[pos]))
							{
								power = (power * 10) + (buffer[pos] - '0');
								pos += 1;
							}
							pos -= 1;			
						}
						else
						{
							if(pr == 1)
							{
								printf("ERROR : Invalid real number at line %d, column %d : invalid character after exponent symbol.\n",line_num, pos);
							}
							error_exist = 1;
						}
					}
					else
					{
						power = 0;
					}
					float num = (float)num1 + ((float)num2/(expo(10,dec)));
					if (power >= 0) 
					{
						num *= expo(10, power);
					}
					else 
					{
						power = -(power);
						num /= expo(10, power);
					}
					if (pr == 1) 
					{
						printf("Line Number : %d, Lexeme : RNUM - %lf\n", line_num, num);
					}
					rnum_no += 1;
					addRNum(num, rnum_no);
				}
				else
				{
					if (pr == 1)
					{
						printf("Line Number : %d, Lexeme : NUM - %d\n", line_num, num1);
					}
					num_no += 1;
					addNum(num1, num_no);
				}
			}
			else
			{
				if ((pos+1 < l) && isAlpha(buffer[pos+1]))
				{
					if (pr == 1)
					{
						printf("ERROR : Invalid Identifier at line %d, column %d : invalid start character (can't be number).\n", line_num, pos);
					}
					error_exist = 1;
					while ((pos < l) && ((isAlpha(buffer[pos]) || buffer[pos] == '_' || isDigit(buffer[pos])) && buffer[pos] != '\n'))
					{
						pos += 1;
					}
				}
				else
				{
					if (pr == 1)
					{
						printf("Line Number : %d, Lexeme : NUM - %d\n", line_num, num1);
					}
					num_no += 1;
					addNum(num1, num_no);
				}
			}
		}			
		else if (lookahead == '+')
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : PLUS - +\n", line_num);
			}
			addSym(PLUS);
		}			
		else if (lookahead == '-')
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : MINUS - -\n", line_num);
			}
			addSym(MINUS);
		}
		else if (lookahead == '*')
		{
			if((pos+1 < l) && buffer[pos+1] == '*')
			{			
				if (pr == 1)
				{
					printf("Line Number : %d, Lexeme : COMMENTMARK - **\n", line_num);
				}
				addSym(COMMENTMARK);
				pos += 2;
				if(comment_flag == 1)
				{
					comment_flag ^= 1;
					lookahead = buffer[pos];
				}
				else
				{
					comment_flag ^= 1;
				}
				while((pos < l) && (!(buffer[pos] == '*' && buffer[pos+1] == '*')))
				{
					if (buffer[pos] == '\n')
					{
						line_num += 1;
					}
					pos += 1;
				}
				if (pos < l && buffer[pos] == '\n')
				{
					line_num += 1;
				}
				pos -= 1;
			}
			else
			{
				if (pr == 1)
				{
					printf("Line Number : %d, Lexeme : MUL - *\n", line_num);
				}
				addSym(MUL);
			}
		}
		else if (lookahead == '/')
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : DIV - /\n", line_num);
			}
			addSym(DIV);
		}
		else if (lookahead == '<')
		{
			if((pos+1 < l) && buffer[pos+1] == '=')
			{
				if (pr == 1)
				{
					printf("Line Number : %d, Lexeme : LE - <=\n", line_num);
				}
				addSym(LE);	
				pos += 1;			
			}
			else if((pos+1 < l) && buffer[pos+1] == '<')
			{
				if((pos+2 < l) && buffer[pos+2] == '<')
				{
					if (pr == 1)
					{
						printf("Line Number : %d, Lexeme : DRIVERDEF - <<<\n", line_num);
					}
					addSym(DRIVERDEF);
					pos += 2;
				}
				else
				{
					if (pr == 1)
					{
						printf("Line Number : %d, Lexeme : DEF - <<\n", line_num);
					}
					addSym(DEF);
					pos += 1;
				}
			}
			else
			{
				if (pr == 1)
				{
					printf("Line Number : %d, Lexeme : LT - <\n", line_num);
				}
				addSym(LT);			
			}
		}
		else if (lookahead == '>')
		{
			if((pos+1 < l) && buffer[pos+1] == '=')
			{
				if (pr == 1)
				{
					printf("Line Number : %d, Lexeme : GE - >=\n", line_num);
				}
				addSym(GE);	
				pos += 1;			
			}
			else if((pos+1 < l) && buffer[pos+1] == '>')
			{
				if((pos+2 < l) && buffer[pos+2] == '>')
				{
					if (pr == 1) 
					{
						printf("Line Number : %d, Lexeme : DRIVERENDDEF - >>>\n", line_num);
					}
					addSym(DRIVERENDDEF);
					pos += 2;
				}
				else
				{
					if (pr == 1) 
					{
						printf("Line Number : %d, Lexeme : ENDDEF - >>\n", line_num);
					}
					addSym(ENDDEF);
					pos += 1;
				}
			}
			else
			{
				if (pr == 1) 
				{
					printf("Line Number : %d, Lexeme : GT - >\n", line_num);
				}
				addSym(GT);			
			}
		}
		else if (lookahead == '=')
		{
			if((pos+1 < l) && buffer[pos+1] == '=')
			{
				if (pr == 1) 
				{
					printf("Line Number : %d, Lexeme : EQ - ==\n", line_num);
				}
				addSym(EQ);
				pos += 1;
			}
			else
			{
				if(pr == 1)
				{
					printf("ERROR : Invalid symbol '=' at line %d, column %d.\n",line_num, pos);
				}
				error_exist = 1;
				pos += 1;
			}
		}
		else if (lookahead == '!')
		{
			if((pos+1 < l) && buffer[pos+1] == '=')
			{			
				if (pr == 1) 
				{
					printf("Line Number : %d, Lexeme : NE - !=\n", line_num);
				}
				addSym(NE);
				pos += 1;
			}
			else
			{
				if(pr == 1)
				{
					printf("ERROR : Invalid symbol '!' at line %d, column %d.\n",line_num, pos);
				}
				error_exist = 1;
			}
		}
		else if (lookahead == ':')
		{
			if((pos+1 < l) && buffer[pos+1] == '=')
			{			
				if (pr == 1)
				{
					printf("Line Number : %d, Lexeme : ASSIGNOP - :=\n", line_num);
				}
				addSym(ASSIGNOP);
				pos += 1;
			}
			else
			{
				if (pr == 1)
				{
					printf("Line Number : %d, Lexeme : COLON - :\n", line_num);
				}
				addSym(COLON);
			}
		}
		else if (lookahead == '.')
		{
			if((pos+1 < l) && buffer[pos+1]=='.')
			{
				if ((pos+2 < l) && isDigit(buffer[pos+2]))
				{
					if (pr == 1)
					{
						printf("Line Number : %d, Lexeme : RANGEOP - ..\n", line_num);
					}
					addSym(RANGEOP);
					pos += 1;
				}
				else
				{
					if (pr == 1)
					{
						printf("ERROR : Invalid Symbol at line %d, column %d. (Had expected range operation)\n", line_num, pos);
					}
					error_exist = 1;
					while(pos < l && buffer[pos] == '.')
					{
						pos += 1;
					}
					pos -= 1;
				}
			}
			else
			{
				if(pr == 1)
				{
					printf("ERROR : Invalid symbol '.' at line %d, column %d.\n",line_num, pos);
				}
				error_exist = 1;
			}
		}
		else if (lookahead == ',')
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : COMMA - ,\n", line_num);
			}
			addSym(COMMA);
		}
		else if (lookahead == ';')
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : SEMICOL - ;\n", line_num);
			}
			addSym(SEMICOL);
		}
		else if (lookahead == '(')
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : BO - (\n", line_num);
			}
			addSym(BO);
		}
		else if (lookahead == ')')
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : BC - )\n", line_num);
			}
			addSym(BC);
		}
		else if (lookahead == '[')
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : SQBO - [\n", line_num);
			}
			addSym(SQBO);
		}
		else if (lookahead == ']')
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : SQBC - ]\n", line_num);
			}
			addSym(SQBC);
		}
		else if(buffer[pos] == '\n')
		{
			line_num += 1;
		}
		pos += 1;

		//ignore whitespaces
		while((pos < l) && (buffer[pos] == ' ' || buffer[pos] == '\t'))
		{
			pos += 1;
		}
		lookahead = buffer[pos];
	}
}

int populateLexemeTable(char *opfile, hashTable table, int pr)
{
	f_read = fopen(opfile,"r");
	first = (tokenInfo *)malloc(sizeof(tokenInfo));
	end = (tokenInfo *)malloc(sizeof(tokenInfo));
	first->df = -1;
	end = first;
	char buffer[BUFFSIZE];
	error_exist = 0;
	while(1)
	{
		strcpy(buffer, getStream());
		if(strcmp(buffer, "") == 0)
		{
			if (pr == 1)
			{
				printf("Line Number : %d, Lexeme : $\n", line_num);
			}
			addDollar();
			break;
		}
		DFA(table, buffer, pr);
	}
	scan = first;
	comment_flag = 0;	
	line_num = 1;
	id_num = 1;
	return error_exist;
}
		
void removeComments(char *testcaseFile, char *cleanFile)
{
	FILE *f_write = fopen(cleanFile, "w");
	f_read = fopen(testcaseFile, "r");
	char buffer[BUFFSIZE];
	strcpy(buffer, "\0");
	comment_flag = 0;
	while(1)
	{
		strcpy(buffer, getStream());
		if(strcmp(buffer, "") == 0)
		{
			break;
		}
		int pos = 0;
		int l = strlen(buffer);
		while(pos < l)
		{
			//ignore content between the comments
			if (comment_flag == 1)
			{
				while((pos < l) && (!(buffer[pos] == '*' && buffer[pos+1] == '*')))
				{
					pos += 1;
				}
			}
			while((pos < l) && (!(buffer[pos] == '*' && buffer[pos+1] == '*')))
			{
				fprintf(f_write, "%c", buffer[pos]);
				pos += 1;
			}
			if(pos != l)
			{
				//found start/end of comment
				pos += 2;
				if(comment_flag == 1)
				{
					comment_flag ^= 1;
					fprintf(f_write, "\n");
				}
				else 
				{
					comment_flag ^= 1;
				}
			}
		}
	}
	fclose(f_write);
}