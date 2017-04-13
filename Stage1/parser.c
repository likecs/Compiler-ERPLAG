/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parser.h"
#include "parserDef.h"

char *token[TSIZE] = 
{
"INTEGER", "REAL", "BOOLEAN", "OF", "ARRAY", "START", "END", "DECLARE", "MODULE", "DRIVER", 
"PROGRAM", "GET_VALUE", "PRINT", "USE", "WITH", "PARAMETERS", "TRUE_", "FALSE_", "TAKES", 
"INPUT", "RETURNS", "AND", "OR", "FOR", "IN", "SWITCH", "CASE", "BREAK", "DEFAULT", "WHILE", 
"PLUS", "MINUS", "MUL", "DIV", "LT", "LE", "GE", "GT", "EQ", "NE", "DRIVERDEF", "DRIVERENDDEF", 
"DEF", "ENDDEF", "COLON", "RANGEOP", "SEMICOL", "COMMA", "ASSIGNOP", "SQBO", "SQBC", "BO", 
"BC", "COMMENTMARK", "ID", "NUM", "RNUM", "$", "e", "null_point"
};

char *lexeme[TSIZE] = 
{
"integer", "real", "boolean", "of", "array", "start", "end", "declare", "module", "driver", 
"program", "get_value", "print", "use", "with", "parameters", "true", "false", "takes", 
"input", "returns", "AND", "OR", "for", "in", "switch", "case", "break", "default", "while", 
"+", "-", "*", "/", "<", "<=", ">", ">=", "==", "!=", "<<<", ">>>", "<<", ">>", ":", "..", 
";", ",", ":=", "[", "]", "(", ")", "**", "", "", "", "$", "e", "null_point"
};

char *nonterminal_map[NTSIZE] = 
{
"program", "moduleDeclarations", "moduleDeclaration", "otherModules", "driverModule", "module", 
"ret", "input_plist", "N1", "output_plist", "N2", "dataType", "type", "moduleDef", "statements", 
"statement", "ioStmt", "var", "whichID", "simpleStmt", "assignmentStmt", "whichStmt", 
"lvalueIDStmt", "lvalueARRStmt", "index_nt", "moduleReuseStmt", "optional", "idList", "N3", 
"expression", "arithmeticOrBooleanExpr", "N7", "AnyTerm", "N8", "arithmeticExpr", "N4", "term_1", 
"N5", "factor", "op1", "op2", "logicalOp", "relationalOp", "declareStmt", "conditionalStmt", 
"caseStmts", "N9", "value", "default_1", "iterativeStmt", "range", "error"
};

int sz[TSIZE];
int sz2[TSIZE];
int vis[TSIZE];
int parse_status = 1;
static int table[NTSIZE][TSIZE];

char *getLexeme(term t) 
{
	char *s = (char *)malloc(TSIZE*sizeof(char));
	if ((int)t < TSIZE)
	{
		strcpy(s, lexeme[(int)t]);
	}
	else 
	{
		strcpy(s, "error");
	}
	return s;
}

char *getNonTerm(non_term t)
{
	char *s = (char *)malloc(TSIZE*sizeof(char));
	strcpy(s, nonterminal_map[(int)t]);
	return s;
}

char *getToken(term t) 
{
	char *s = (char *)malloc(TSIZE*sizeof(char));
	strcpy(s, token[(int)t]);
	return s;
}

void initParseTable()
{
	int i, j;
	for(i = 0; i < NTSIZE; ++i)
	{
		for(j = 0; j < TSIZE; ++j)
		{
			table[i][j] = -1;
		}
	}
}

void fillFirstRules(gNode n, int idx, Grammar G)
{
	int i;
	for(i = 0; i < 20; ++i)
	{
		if (first[(int)n->ge.nt][i] == -1)
		{
			break;
		}
		if (first[(int)n->ge.nt][i] == (term)e)
		{
			if (n->next == NULL)
			{
				fillFollowRules(G[idx].t, idx, G);
			}
			else
			{
				if (n->next->t == terminal)
				{
					table[(int)G[idx].t][(int)n->next->ge.t] = idx;
				}
				else
				{
					fillFirstRules(n->next, idx, G);
				}
			}
		} 
		else
		{
			table[(int)G[idx].t][(int)first[(int)n->ge.nt][i]] = idx;
		}
	}
}

void fillFollowRules(non_term nt, int idx, Grammar G)
{
	int i;
	for(i = 0; i < 20; ++i)
	{
		if (follow[(int)nt][i] == -1)
		{
			break;
		}
		table[(int)G[idx].t][(int)follow[(int)nt][i]] = idx;
	}
}

void createParseTable(Grammar G)
{
	initParseTable();
	int i;
	for(i = 0; i < RULECNT; ++i)
	{
		if (G[i].top->t == terminal)
		{
			if (G[i].top->ge.t != (term)e)
			{
				table[(int)G[i].t][(int)G[i].top->ge.t] = i;
			}
			else
			{
				fillFollowRules(G[i].t, i, G);
			}
		}
		else
		{
			fillFirstRules(G[i].top, i, G);
		}
	}
}

gNode initGrammarNode(gElement ge, tag t)
{
	gNode temp = (gNode)malloc(sizeof(struct gNode));
	if (temp != NULL)
	{
		temp->ge = ge;
		temp->t = t;
		temp->next = NULL;
	}
	return temp;
}

Grammar createGrammar()
{
	int i, j, k;
	FILE *fp = fopen("Grammar.txt", "r");
	char data[150], temp[150];
	Grammar G = (Grammar)malloc(RULECNT * sizeof(gHead));
	for(i = 0; i < RULECNT; ++i)
	{
		fgets(data, 150, fp);
		int len = strlen(data), eq = 0;
		for(j = 0; j < len; ++j)
		{
			if (data[j] == '<')
			{
				int pos = 0;
				j += 1;
				while(j < len && data[j] != '>')
				{
					temp[pos] = data[j];
					j += 1;
					pos += 1;
				}
				temp[pos] = '\0';
				if (eq == 0)
				{
					for(k = 0; k < NTSIZE; ++k)
					{
						if (strcmp(nonterminal_map[k], temp) == 0)
						{
							G[i].t = (non_term)k;
							G[i].top = NULL;
							break;
						}
					}
				}
				else
				{
					for(k = 0; k < TSIZE; ++k)
					{
						if (strcmp(nonterminal_map[k], temp) == 0)
						{
							gNode s = initGrammarNode((gElement)(non_term)k, non_terminal);
							gNode last = G[i].top;
							if (last == NULL)
							{
								G[i].top = s;
							}
							else
							{
								while(last->next != NULL)
								{
									last = last->next;
								}
								last->next = s;
							}
							break;
						}
					}
				}
			}
			else if (data[j] == '=')
			{
				eq = 1;
			}
			else if (data[j] == ' ')
			{
				continue;
			}
			else
			{
				int pos = 0;
				while(j < len && data[j] != ' ' && data[j] != '\n' && data[j] != '\r')
				{
					temp[pos] = data[j];
					j += 1;
					pos += 1;
				}
				temp[pos] = '\0';
				for(k = 0; k < TSIZE; ++k)
				{
					if (strcmp(token[k], temp) == 0)
					{
						gNode s = initGrammarNode((gElement)(term)k, terminal);
						gNode last = G[i].top;
						if (last == NULL)
						{
							G[i].top = s;
						}
						else
						{
							while(last->next != NULL)
							{
								last = last->next;
							}
							last->next = s;
						}
						break;
					}
				}
			}
		}
	}
	fclose(fp);
	return G;
}

void printGrammar(Grammar G)
{
	int i;
	for(i = 0; i < RULECNT; ++i)
	{
		printf("<%s> =", nonterminal_map[(int)G[i].t]);
		gNode temp = G[i].top;
		while(temp != NULL)
		{
			if (temp->t == terminal)
			{
				printf(" %s", token[(int)temp->ge.t]);
			}
			else
			{
				printf(" <%s>", nonterminal_map[(int)temp->ge.nt]);
			}
			temp = temp->next;
		}
		printf("\n");
	}
}

void addFirst(term q, int ind)
{
	int i;
	for(i=0; i < sz[ind]; i++)
	{
		if(first[ind][i] == q)
		{
			break;
		}
	}
	if(i == sz[ind])
	{
		first[ind][sz[ind]] = q;
		sz[ind]++;
	}
}

void addFollow(term q, int ind)
{
	int i;
	for(i=0;i < sz2[ind]; i++)
	{
		if(follow[ind][i] == q)
		{
			break;
		}
	}
	if(i == sz2[ind])
	{
		follow[ind][sz2[ind]] = q;
		sz2[ind]++;
	}
}

void dfs(non_term node, Grammar G)
{
	if(vis[(int)node])
	{
		return;
	}
	int i, j;
	for(i = 0; i < RULECNT; i++)
	{
		if(G[i].t == node)
		{
			gNode cur = G[i].top;
			while(1)
			{
				if(cur->t == terminal)
				{
					addFirst(cur->ge.t, (int)node);
					break;
				}
				dfs(cur->ge.nt,G);
				int nul = 0;
				int q = (int)cur->ge.nt;
				for(j = 0; ; ++j)
				{
					if (first[q][j] == -1)
					{
						break;
					}
					if(strcmp(token[(int)first[q][j]],"e")==0)
					{
						nul = 1;
					}
					else
					{
						addFirst(first[q][j], (int)node);
					}	
				}
				if(nul == 1)
				{
					cur = cur->next;
					if(cur == NULL)
					{
						addFirst((term)e,(int)node);
						break;
					}
				}
				else
				{
					break;
				}
			}		
		}
	}
	vis[(int)node] = 1;
}

void findFirst(Grammar G)
{
	int i, j;
	for(i = 0; i < TSIZE; ++i)
	{
		sz[i] = 0;
		vis[i] = 0;
		for(j = 0; j < 20; ++j)
		{
			first[i][j] = -1;
		}
	}	
	for(i = 0; i < RULECNT; ++i)
	{
		non_term tmp=G[i].t;
		if(!vis[(int)tmp])
		{
			dfs(tmp,G);
			vis[(int)tmp] = 1;
		}
	}
	// printf("FIRST : \n");
	// for(i = 0; i < TSIZE; ++i)
	// {
	// 	printf("%s : ", nonterminal_map[i]);
	// 	for(j = 0; j < 20; ++j)
	// 	{
	// 		if (first[i][j] == -1)
	// 		{
	// 			break;
	// 		}
	// 		printf("%s ", token[first[i][j]]);
	// 	}
	// 	printf("\n");
	// }
}

void findFollow(Grammar G)
{
	int i, j, k;
	for(i = 0; i < TSIZE; ++i)
	{
		sz2[i] = 0;
		for(j = 0; j < 20; ++j)
		{
			follow[i][j] = -1;
		}
	}
	addFollow((term)$, 0);
	for(k = 0; k < 2; ++k)
	{
		for(i = 0; i < RULECNT; ++i)
		{
			gNode cur = G[i].top, nxt;
			non_term par = G[i].t;
			while(cur != NULL)
			{
				if(cur->t == terminal)
				{
					cur = cur->next;
				}
				else
				{
					nxt = cur->next;
					while(nxt != NULL)
					{
						if(nxt->t == terminal)
						{
							addFollow(nxt->ge.t, (int)(cur->ge.nt));
							break;
						}
						else
						{
							int flg = 0;
							int q = (int)(nxt->ge.nt);
							for(j = 0; j < sz[q]; ++j)
							{
								if(strcmp(token[(int)first[q][j]], "e") == 0)
								{
									flg=1;
								}
								else
								{
									addFollow(first[q][j],(int)(cur->ge.nt));
								}	
							}
							if(!flg)
							{
								break;
							}
							else
							{
								nxt=nxt->next;
							}
						}	
					}
					if(nxt == NULL)
					{
						int q = (int)par;
						for(j = 0; j < sz2[q]; ++j)
						{
							addFollow(follow[q][j],(int)(cur->ge.nt));
						}
					}
					cur = cur->next;
				}
			}
		}
	}
	// printf("FOLLOW : \n");
	// for(i = 0; i < TSIZE; ++i)
	// {
	// 	printf("%s : ", nonterminal_map[i]);
	// 	for(j = 0; j < 20; ++j)
	// 	{
	// 		if (follow[i][j] == -1)
	// 		{
	// 			break;
	// 		}
	// 		printf("%s ", token[follow[i][j]]);
	// 	}
	// 	printf("\n");
	// }
}

pNode returnCurrent(pNode p)
{
	while (p != NULL && (p->child.size == p->no_of_child))
	{
		p = p->parent;
	}
	return p;
}

Queue initQueue()
{
	Queue q;
	q.front = NULL;
	q.back = NULL;
	q.size = 0;
	return q;
}

Queue enqueue(pNode q, pNode p)
{
	p->parent = q;
	if (q->child.size == 0)
	{
		q->child.front = p;
		q->child.back = q->child.front;
	}
	else
	{
		q->child.back->next = p;
		q->child.back = p;
		if (q->child.front->next == NULL)
		{
			q->child.front->next = q->child.back;
		}
	}
	q->child.size += 1;
	return q->child;
}

PTStack pop(PTStack s)
{
	PTStack temp;
	gNode delme = s.top;
	temp.top = s.top->next;
	temp.size = s.size-1;
	free(delme);
	return temp;
}

PTStack mergeStack(gHead T, PTStack s)
{
	//For given rule in gHead push the reverse of RHS in stack after poping top element
	s = pop(s);
	PTStack ms;
	ms.top = NULL;
	ms.size = 0;
	gNode store;
	while (T.top != NULL)
	{ 
		store = T.top;
		T.top = T.top->next;
		if (ms.size == 0)
		{
			store->next = NULL;
		}
		else 
		{
			store->next = ms.top;
		}
		ms.top = store;
		ms.size++;
	}
	while (ms.size > 0)
	{
		store = ms.top;
		ms.top = ms.top->next;
		ms.size--;
		store->next = s.top;
		s.top = store;
		s.size++;
	}
	return s;
}

pNode createNonTerminal(pNode Parent, non_term nt, tag t, int no_of_child, int level)
{
	pNode temp = (pNode)malloc(sizeof(struct pNode));
	temp->parent = Parent;
	temp->t = t;
	temp->e.nt = nt;
	temp->level = level;
	temp->no_of_child = no_of_child;
	temp->child = initQueue();
	temp->next = NULL;
	temp->token_link = NULL;
	return temp;
}

pNode createTerminal(pNode Parent, term term_1, tag t, int level, tokenInfo* token)
{     
	pNode temp = (pNode)malloc(sizeof(struct pNode));
	temp->parent = Parent;
	temp->e.t = term_1;
	temp->t = t;
	temp->level = level;
	temp->no_of_child = 0;
	temp->child = initQueue();
	temp->next = NULL;  
	temp->token_link = token;
	return temp;
}

parseTree initParseTree()
{
	parseTree pt;
	pt.root = NULL;
	pt.height = 0;
	pt.c_node = pt.root;
	return pt;
}

parseTree parseInputSourceCode(char * testCaseFile, hashTable h, Grammar G, int print)
{
	FILE *fp;
	fp = fopen(testCaseFile, "r");

	int i, flag = 0, parse_status = 1;
	parseTree P = initParseTree();
	
	// populating Symbol Table and creating the Token file
	int error_lexer = populateLexemeTable(testCaseFile,h,print);
	if (error_lexer == 1)
	{
		parse_status = 0;
		printf("Found errors in lexer. So parsing can't be done\n");
		return P;
	}

	// Initializing stack to contain the $, start Symbol= program;
	PTStack stack;
	stack.top = initGrammarNode((gElement)(term)$,terminal);
	stack.size = 1;

	gNode duplicate;
	duplicate = initGrammarNode((gElement)(non_term)program,non_terminal);
	duplicate->next = stack.top;
	stack.top = duplicate;
	stack.size += 1;

	char *stackToken = "\0";
	char *local_token = "\0";
	char *local_lexeme = "\0";
	pNode temp;
	tokenInfo* currentToken;
	currentToken = getNextToken();
	int rule, stacksize_i, stacksize_f;

	while((currentToken!=NULL) && (((stack.top->t == terminal) && (stack.top->ge.t != $)) || (stack.top->t == non_terminal)))
	{
		//some changes occured in Grammer while handling $ and e
		G = createGrammar();
		if (stack.top->t == terminal)
		{ 
			stackToken = getToken(stack.top->ge.t);
			if (stack.top->ge.t==e)
			{
				stack = pop(stack);
				temp = createTerminal(P.c_node, (term)e, (tag)terminal, P.c_node->level+1, NULL);   
				P.c_node->child = enqueue(P.c_node,temp);
				if (P.c_node->level == P.height)
				{
					P.height = P.c_node->level + 1;
				}
				P.c_node = returnCurrent(P.c_node);
			}
			else
			{ 
				if (stack.top->ge.t == currentToken->s)
				{ 
					stack = pop(stack);
					temp = createTerminal(P.c_node, (term)currentToken->s, (tag)terminal, P.c_node->level+1, currentToken);
					P.c_node->child = enqueue(P.c_node,temp);
					if (P.c_node->level == P.height)
					{
						P.height = P.c_node->level+1;
					}
					P.c_node = returnCurrent(P.c_node);
					currentToken=getNextToken();
				}
				else
				{
					local_token = getToken(currentToken->s);
					printf("\n%d\n", stack.top->ge.t);
					stackToken = getToken(stack.top->ge.t);
					if (currentToken->s == NUM)
					{
						printf("\nThe token (%s) for lexeme (%d) does not match at line no. %d. The expected token here is %s", local_token,currentToken->val.i,currentToken->lno,stackToken);
					}
					else if (currentToken->s == RNUM)
					{
						printf("\nThe token (%s) for lexeme (%f) does not match at line no. %d. The expected token here is %s", local_token,currentToken->val.f,currentToken->lno,stackToken);
					}
					else if (currentToken->s == ID)
					{
						printf("\nThe token (%s) for lexeme (%s) does not match at line no. %d. The expected token here is %s", local_token,currentToken->val.s,currentToken->lno,stackToken);
					}
					else 
					{
						local_lexeme = getLexeme(currentToken->s);
						printf("\nThe token (%s) for lexeme (%s) does not match at line no. %d. The expected token here is %s", local_token,local_lexeme,currentToken->lno,stackToken);
					}
					printf("\n%d\n",stack.top->ge.t);
					flag = 1;
					printf("\nParsing not successful\n");
					break;
				}
			}
		}
		else
		{ 
			stackToken = getNonTerm(stack.top->ge.nt);
			rule = table[(int)stack.top->ge.nt][(int)currentToken->s];
			if (rule != -1)
			{ 
				stacksize_i = stack.size-1;
				stack = (PTStack)mergeStack(G[rule],stack);
				stacksize_f = stack.size;
				if (P.c_node == NULL)
				{
					temp = createNonTerminal(P.c_node,(non_term)G[rule].t,(tag)non_terminal,stacksize_f-stacksize_i,0);
					P.root = temp;
					P.c_node = temp;
				}
				else
				{
					temp = createNonTerminal(P.c_node,(non_term)G[rule].t,(tag)non_terminal,stacksize_f-stacksize_i,P.c_node->level+1);
					P.c_node->child = enqueue(P.c_node,temp);
					if (P.c_node->level == P.height)
					{
						P.height = P.c_node->level+1;
					}
					P.c_node = P.c_node->child.back;
				}
			}
			else
			{ 
				local_token = getToken(currentToken->s);
				if (currentToken->s == NUM)
				{
					printf("\nThe token (%s) for lexeme (%d) does not match at line no. %d. The expected token here are %s", local_token,currentToken->val.i,currentToken->lno,stackToken);
				}
				else if (currentToken->s == RNUM)
				{
					printf("\nThe token (%s) for lexeme (%f) does not match at line no. %d. The expected token here are %s", local_token,currentToken->val.f,currentToken->lno,stackToken);
				}
				else if (currentToken->s == ID) 
				{
					printf("\nThe token (%s) for lexeme (%s) does not match at line no. %d. The expected token here are %s", local_token,currentToken->val.s,currentToken->lno,stackToken);
				}
				else 
				{
					local_lexeme = getLexeme(currentToken->s);
					printf("\nThe token (%s) for lexeme (%s) does not match at line no. %d. The expected token here is %s", local_token,local_lexeme,currentToken->lno,stackToken);
				}
				for (i = 0; i < TSIZE; ++i)
				{
					if (table[(int)stack.top->ge.nt][i] != -1)
					{
						stackToken = getToken(i);
						printf("\n %s",stackToken);
					}
				}
				printf("\nParsing not successful\n");
				flag = 1;
				break;
			}
		}
	}
	if(flag == 0)
	{
		printf("\nParsing successful\n");
	}
	else
	{
		parse_status = 0;
	}
	return P;
}

void printNode(FILE *fp, pNode P)
{
	char *p, *s, *eps, *dash, *yes, *no;
	eps = (char *)"e";
	dash = (char *)"---";
	yes = (char *)"y";
	no = (char *)"n";
	if (P->no_of_child == 0)
	{
		if (P->e.t != e)
		{
			s = getToken(P->token_link->s);
			p = getNonTerm(P->parent->e.nt);
			if (P->e.t == NUM)
			{
				fprintf(fp,"\n%-15d:%-5d:%-12s:%-10d:%-25s:%-12s:%-20s",P->token_link->val.i, P->token_link->lno, s, P->token_link->val.i, p, yes, s);
			}
			else 
			{
				if (P->e.t == RNUM)
				{
					fprintf(fp,"\n%-15f:%-5d:%-12s:%-10f:%-25s:%-12s:%-20s",P->token_link->val.f, P->token_link->lno, s, P->token_link->val.f, p, yes, s);
				}
				else
				{
					if (P->e.t == ID)
					{
						fprintf(fp,"\n%-15s:%-5d:%-12s:%-10s:%-25s:%-12s:%-20s",P->token_link->val.s, P->token_link->lno, s, no, p, yes, s); 
					}
					else
					{
						char *lex = getLexeme(P->token_link->s);
						fprintf(fp,"\n%-15s:%-5d:%-12s:%-10s:%-25s:%-12s:%-20s",lex, P->token_link->lno, s, no, p, yes, s);
					}
				}
			}
		}
		else
		{
			p = getNonTerm(P->parent->e.nt);
			fprintf(fp,"\n%-15s:%-5s:%-12s:%-10s:%-25s:%-12s%-20s",eps, no, no, no, p, yes, no);
		}
	}
	else
	{
		if (P->parent != NULL)
		{
			p = getNonTerm(P->parent->e.nt);
		}
		else
		{
			p = "ROOT";
		}
		s = getNonTerm(P->e.nt);
		fprintf(fp,"\n%-15s:%-5s:%-12s:%-10s:%-25s:%-12s:%-20s",no, no, no, no, p, no, s);
	}
}

void printParseTreeQueue(FILE *fp, pNode p)
{
	if (p->no_of_child == 0)
	{
		printNode(fp, p);
	}
	else 
	{
		int br = 0;
		pNode trav = p->child.front;
		while(trav != NULL)
		{
			pNode doit = trav;
			pNode store = trav->next;
			printParseTreeQueue(fp, doit);
			trav = store;
			br = 1;
			break;
		}
		//to ensure we found the leftmost child and it is not NULL
		assert(br == 1);
		printNode(fp, p);
		while(trav != NULL)
		{
			pNode doit = trav;
			pNode store = trav->next;
			printParseTreeQueue(fp, doit);
			trav = store;
		}
	}
}

void printParseTree(char *outfile, char * testCaseFile, hashTable h, Grammar G)
{
	parseTree Tree = parseInputSourceCode(testCaseFile, h, G, 0);
	if (Tree.height == 0)
	{
		return; 
	}
	FILE *fp;
	fp = fopen(outfile, "w");
	fprintf(fp,"%-15s:%-5s:%-12s:%-10s:%-25s:%-12s:%-20s","lexCurrentNode", "lNo", "token", "valIfNo", "pNode", "IsLeaf(y/n)", "nodeSymbol");
	printParseTreeQueue(fp,Tree.root);
	printf("%s successfully created.\n", outfile);
	fclose(fp);
}