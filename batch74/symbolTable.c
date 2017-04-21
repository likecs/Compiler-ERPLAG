#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"
#include "symbolDef.h"
#include "symbolTable.h"

static int id_num = 0;

int getBytes(term s)
{
	if(s == INTEGER)
	{
		return 2;
	}
	else if(s == REAL)
	{
		return 4;
	}
	else
	{
		return 1;
	}
}

int checkModuleReturn(IDEntry* ctrl_var, tokenInfo* token, hashTable2 tableID)
{
	int flag = 1;
	while(token->s != START)
	{
		token = token->n;
	}
	token = token->n;
	//check till function definition ends
	//do not use "END" as marker symbol as for/while/switch can exist inside function
	while(token != NULL && token->s != DEF && token->s != DRIVERDEF)
	{
		if(token->s != ID)
		{
			token = token->n;
			continue;
		}
		if(strcmp(ctrl_var->entity.ivar.word, token->val.s) != 0)
		{
			token = token->n;
			continue;
		}
		//idntifier matched with one in output parameter list
		//if user take an input value
		if(token->prev != NULL && token->prev->prev != NULL && token->prev->prev->s == GET_VALUE)
		{		
			flag = 0;
			break;
		}
		token = token->n;
		//or user assigns an input value
		while(token != NULL && token->s != SEMICOL)
		{
			if(token->s == ASSIGNOP)
			{
				flag = 0;
				break;
			}
			token = token->n;
		}
		if(flag == 0)
		{
			break;
		}
	}
	return flag;
}	

void checkFor(IDEntry* ctrl_var, tokenInfo* token, hashTable2 tableID, int print)
{
	int flag;
	while(token != NULL && token->s != START)
	{
		token = token->n;
	}
	token = token->n;
	//check till for loop ends
	while(token != NULL && token->s != END)
	{
		flag = 0;
		if(token->s != ID)
		{
			token = token->n;
			continue;
		}
		if(strcmp(ctrl_var->entity.ivar.word, token->val.s) != 0)
		{
			token = token->n;
			continue;
		}
		//identifier matched with one in for loop declaration
		assert(token->prev->prev != NULL);
		if(token->prev->prev->s == GET_VALUE)
		{
			if(print == 1)
			{
				printf("\nERROR : The for statement must not redefine the variable \"%s\" at line number %d", ctrl_var->entity.ifunc.word, token->lno);
			}
			semantic_status = 0;
			token = token->n;
			continue;
		}
		//check if variable is not of array type
		tokenInfo* go = token;
		while(go->prev->s != SEMICOL && go->prev->s != START)
		{
			go = go->prev;
		}
		if(go->s == ID && go->n->s == SQBO)
		{
			flag = 1;
		}
		token = token->n;
		while(token != NULL && token->s != SEMICOL)
		{
			if(token->s == ASSIGNOP && flag == 0)
			{
				if(print == 1)
				{
					printf("\nERROR : The for statement must not redefine the variable \"%s\" at line number %d", ctrl_var->entity.ifunc.word, token->lno);
				}
		 		semantic_status = 0;
				while(token->s != SEMICOL)
				{
					token = token->n;
				}
				break;
			}
			token = token->n;
		}
	}
}

void createIDTable(tokenInfo* token, totalScopeList* scope_tot, hashTable2 tableID, hashTable2 tableFunc, int print)
{
	store2 res;
	scopeChain* TREE = (scopeChain*)malloc(sizeof(scopeChain));
	TREE->scope = 0;
	TREE->next = NULL;
	TREE->prev = NULL;
	TREE->in_cond = 0;
	TREE->in_loop = 0;
	int univ_scope = 0;
	char current_func[20];
	strcpy(current_func, " ");
	char name;
	int i = -2;
	int default_flag = 0;
	int switch_type;
	IDEntry* for_start = NULL;
	int switch_error[3] = {-1, -1, -1};	
	int switch_scope_end;
	int switch_scope_start;
	int depth_in_scope = 0, offset_in_function = 0;

	while(token != NULL)
	{
		if(token->s == MODULE)
		{
			assert(token->n != NULL);
			res = findScope(tableFunc, token->n->val.s, TREE->scope, 1);	
			if(token->prev != NULL && token->prev->s != USE)
			{
				// function declaration/definition starts, so current function name changes
				if(token->prev->s == DEF)
				{
					depth_in_scope = 0;
					offset_in_function = 0;
					strcpy(current_func, token->n->val.s);
				}
				//function name not found
				if(res.code == -1)			
				{
					token = token->n;
					IDEntry* par_data = (IDEntry*)malloc(sizeof(IDEntry));
					strcpy(par_data->entity.ifunc.word, token->val.s);
					if(token->prev->prev->s == DECLARE)
					{
						par_data->entity.ifunc.status = declared;
						par_data->entity.ifunc.inputList = NULL;
						par_data->entity.ifunc.outputList = NULL;
					}
					else
					{
						par_data->entity.ifunc.scope = TREE->scope;
						par_data->entity.ifunc.lno = token->lno;
						par_data->entity.ifunc.status = defined;
						par_data->entity.ifunc.inputList = NULL;
						par_data->entity.ifunc.outputList = NULL;
					}
					addScope(tableFunc, par_data, 1);
				}
				else
				{
					assert(token->prev != NULL);
					if(token->prev->s == DEF)
					{
						token = token->n;
						if(res.node->entity.ifunc.status == declared)
						{
							res.node->entity.ifunc.scope = TREE->scope;
							res.node->entity.ifunc.lno = token->lno;
							res.node->entity.ifunc.status = defined;
						}
						else
						{
							if (print)
							{
								printf("\nERROR : Overloading of module \"%s\" at line number %d not allowed",current_func,token->lno);
							}
			 				semantic_status = 0;
						}
					}
					else if(token->prev->s == DECLARE)
					{
						token = token->n;
						if (print)
						{
							printf("\nERROR : Redundant declaration of module \"%s\" at line number %d",current_func,token->lno);
						}
			 			semantic_status = 0;
					}
				}
			}
			else
			{
				//Function calling statements
				token = token->n;
				if(strcmp(current_func, token->val.s) == 0)
				{
					if (print)
					{
						printf("\nERROR : Function \"%s\" at line number %d cannot be invoked recursively",current_func,token->lno);
					}
			 		semantic_status = 0;
				}			
				else if(res.code == -1)				
				{	
					if (print)
					{
						printf("\nERROR : Undeclared module \"%s\" at line number %d",token->val.s, token->lno);
					}
		 			semantic_status = 0;
				}
				else
				{
					res.node->entity.ifunc.used = 1;
				}		
			}
		}		
		else if(token->s == INPUT)
		{
			token = token->n->n;
			while (token != NULL && token->s != SQBC)
			{
				//ignore comma in input list of function
				if(token->s == COMMA)
				{
					token = token->n;
				}
				IDEntry* par_data = (IDEntry*)malloc(sizeof(IDEntry));
				strcpy(par_data->entity.ivar.word, token->val.s);
				par_data->entity.ivar.code = id_num++;
				par_data->entity.ivar.lno = token->lno;
				//ignore the colon
				assert(token->n != NULL);
				token = token->n->n;
				if(token->s == ARRAY)
				{
					par_data->entity.ivar.v_a = 1;
					//ignore opening square bracket
					token = token->n->n;
					par_data->entity.ivar.var.a.s_idx = token->val.i;
					//ignore range operator
					token = token->n->n;
					par_data->entity.ivar.var.a.e_idx = token->val.i;
					//ignore closing bracket and "of"
					token = token->n->n->n;
					assert(token != NULL);
					par_data->entity.ivar.var.a.type = token->s;
					par_data->entity.ivar.bytes = getBytes(token->s);
				}
				else
				{
					par_data->entity.ivar.v_a = 0;
					par_data->entity.ivar.var.v.type = token->s;
					par_data->entity.ivar.bytes = getBytes(token->s);
				}
				par_data->entity.ivar.scope = TREE->scope;
				res = findScope(tableID, par_data->entity.ivar.word, TREE->scope, 0);
				if(res.code >= 0)
				{
					if (print)
					{
						printf("\nERROR : Repeat declaration of identifier \"%s\" in function input list at line number %d",par_data->entity.ivar.word, par_data->entity.ivar.lno);
					}
			 		semantic_status = 0;
				}
				else
				{	
					strcpy(par_data->entity.ivar.func_name, current_func);
					par_data->entity.ivar.depth = depth_in_scope + 1;
					par_data->entity.ivar.offset = offset_in_function;
					addScope(tableID, par_data, 0);
					int bytes = par_data->entity.ivar.bytes;
					if (par_data->entity.ivar.v_a == 1) 
					{
						bytes *= (par_data->entity.ivar.var.a.e_idx - par_data->entity.ivar.var.a.s_idx + 1);
					}
					offset_in_function += bytes;
					res = findScope(tableFunc,current_func, TREE->scope, 1);
					//create link list of parameter values
					if(res.node->entity.ifunc.inputList == NULL)
					{
						parameter* root = (parameter*)malloc(sizeof(parameter));
						if(par_data->entity.ivar.v_a == 0)
						{
							root->v_a = 0;
							root->var.v.type = par_data->entity.ivar.var.v.type;
							root->next = NULL;
						}
						else
						{
							root->v_a = 1;
							root->var.a.type = par_data->entity.ivar.var.a.type;
							root->var.a.s_idx = par_data->entity.ivar.var.a.s_idx;
							root->var.a.e_idx = par_data->entity.ivar.var.a.e_idx;
							root->next = NULL;
						}
						res.node->entity.ifunc.inputList = root;
					}
					else
					{
						parameter* root = res.node->entity.ifunc.inputList;
						while(root->next!=NULL)
						{
							root = root->next;
						}
						root->next = (parameter*)malloc(sizeof(parameter));
						if(par_data->entity.ivar.v_a==0)
						{
							root->next->v_a = 0;
							root->next->var.v.type = par_data->entity.ivar.var.v.type;
							root->next->next = NULL;
						}
						else
						{
							root->next->v_a=1;
							root->next->var.a.type = par_data->entity.ivar.var.a.type;
							root->next->var.a.s_idx = par_data->entity.ivar.var.a.s_idx;
							root->next->var.a.e_idx = par_data->entity.ivar.var.a.e_idx;
							root->next->next = NULL;
						}
					}
				}
				assert(token != NULL);
				token = token->n;
			}
		}
		else if(token->s == RETURNS)
		{
			token = token->n->n;
			while(token != NULL && token->s != SQBC)
			{
				//ignore comma in outputlist of function
				if(token->s == COMMA)
				{
					token = token->n;
				}
				IDEntry* par_data = (IDEntry*)malloc(sizeof(IDEntry));
				strcpy(par_data->entity.ivar.word, token->val.s);
				par_data->entity.ivar.code = id_num++;
				par_data->entity.ivar.lno = token->lno;
				//ignore the colon
				assert(token->n != NULL);
				token = token->n->n;
				if (token->s == ARRAY)
				{
					//ignore opening square bracket
					token = token->n->n;
					//ignore range operator
					token = token->n->n;
					//ignore closing bracket and "of"
					token = token->n->n->n;
					assert(token != NULL);
					if (print)
					{
						printf("\nERROR : Output list of function can't contain array as parameter\n");
					}
					semantic_status = 0;
				}
				else
				{
					par_data->entity.ivar.v_a = 0;
					par_data->entity.ivar.var.v.type = token->s;
					par_data->entity.ivar.bytes = getBytes(token->s);
					par_data->entity.ivar.scope = TREE->scope;
					res = findScope(tableID, par_data->entity.ivar.word, 0, TREE->scope);
					if(res.code >= 0)
					{
						if (print)
						{
							printf("\nERROR : Repeat declaration of identifier \"%s\" at line number %d",par_data->entity.ivar.word,par_data->entity.ivar.lno);
						}
				 		semantic_status = 0;
					}
					else
					{	
						strcpy(par_data->entity.ivar.func_name, current_func);
						par_data->entity.ivar.depth = depth_in_scope + 1;
						par_data->entity.ivar.offset = offset_in_function;
						addScope(tableID, par_data, 0);
						int bytes = par_data->entity.ivar.bytes;
						if (par_data->entity.ivar.v_a == 1) 
						{
							bytes *= (par_data->entity.ivar.var.a.e_idx - par_data->entity.ivar.var.a.s_idx + 1);
						}
						offset_in_function += bytes;
						int invalid = checkModuleReturn(par_data, token, tableID);
						if(invalid == 1)
						{
							if (print)
							{
								printf("\nERROR : The variable \"%s\" returned by the function \"%s\" is not assigned any value within the function definition", par_data->entity.ivar.word, current_func);
							}
							semantic_status = 0;
						}
						res = findScope(tableFunc,current_func, TREE->scope, 1);
						//create link list of parameter values
						if(res.node->entity.ifunc.outputList == NULL)
						{
							parameter* root = (parameter*)malloc(sizeof(parameter));
							assert(par_data->entity.ivar.v_a == 0);
							root->v_a = 0;
							root->var.v.type = par_data->entity.ivar.var.v.type;
							root->next = NULL;
							res.node->entity.ifunc.outputList = root;
						}
						else
						{
							parameter* root= res.node->entity.ifunc.outputList;
							while(root->next != NULL)
							{
								root = root->next;
							}
							root->next=(parameter*)malloc(sizeof(parameter));
							assert(par_data->entity.ivar.v_a == 0);
							root->next->v_a=0;
							root->next->var.v.type=par_data->entity.ivar.var.v.type;
							root->next->next=NULL;
						}
					}
				}
				token = token->n;
			}
		}
		else if(token->s == DECLARE)
		{
			term type; 
			int V_A;
			int s_idx;
			int e_idx;
			assert(token->n != NULL);
			//declaration of variables as module declaration checked before
			if(token->n->s != MODULE)
			{
				token = token->n;
				IDEntry* par_data = (IDEntry*)malloc(sizeof(IDEntry));
				strcpy(par_data->entity.ivar.word, token->val.s);
				par_data->entity.ivar.code = id_num++;
				par_data->entity.ivar.lno = token->lno;
				//ignore all the variable names for now to get datatype first
				//information updated later
				int ctr = -2;	//as ctr reaches 0 only when atleast 2 variables are declared
				while(token != NULL && token->s != COLON)
				{
					token = token->n;
					ctr += 1;
				}
				//ignore colon
				token = token->n;
				if(token->s == ARRAY)
				{
					V_A = 1;
					par_data->entity.ivar.v_a = 1;
					tokenInfo *traverse = token;
					//ignore opening square brackets
					traverse = traverse->n->n;
					s_idx = traverse->val.i;
					par_data->entity.ivar.var.a.s_idx = s_idx;
					//ignore range operator
					traverse = traverse->n->n;
					e_idx = traverse->val.i;
					par_data->entity.ivar.var.a.e_idx = e_idx;
					//ignore closing bracket and "of"
					traverse = traverse->n->n->n;
					assert(traverse != NULL);
					type = traverse->s;
					par_data->entity.ivar.var.a.type = type;
					par_data->entity.ivar.bytes = getBytes(type);
				}
				else
				{
					V_A = 0;
					par_data->entity.ivar.v_a = 0;
					type = token->s;
					par_data->entity.ivar.var.v.type = token->s;
					par_data->entity.ivar.bytes = getBytes(type);
				}
				par_data->entity.ivar.scope = TREE->scope;
				//process 1st variable
				res = findScope(tableID, par_data->entity.ivar.word, TREE->scope, 0);
				if(res.code >= 0)
				{
					if (print)
					{
						printf("\nERROR : Repeat declaration of identifier \"%s\" at line number %d",par_data->entity.ivar.word, par_data->entity.ivar.lno);
					}
		 			semantic_status = 0;
				}
				else
				{
					strcpy(par_data->entity.ivar.func_name, current_func);
					par_data->entity.ivar.depth = depth_in_scope;
					par_data->entity.ivar.offset = offset_in_function;
					addScope(tableID, par_data, 0);
					int bytes = par_data->entity.ivar.bytes;
					if (par_data->entity.ivar.v_a == 1) 
					{
						bytes *= (par_data->entity.ivar.var.a.e_idx - par_data->entity.ivar.var.a.s_idx + 1);
					}
					offset_in_function += bytes;
				}
				token = token->prev;
				assert(token->s == COLON);
				while(ctr > 0)
				{
					token = token->prev;
					ctr -= 1;
				}
				//ctr = 0 check to ensure this executes only in case of multiple variable declarations
				while(ctr == 0 && token->s != COLON)
				{
					if (token->s == COMMA)
					{
						token = token->n;
						continue;
					}
					IDEntry* par_data = (IDEntry*)malloc(sizeof(IDEntry));					
					strcpy(par_data->entity.ivar.word, token->val.s);
					if(V_A == 1)
					{
						par_data->entity.ivar.v_a = 1;
						par_data->entity.ivar.var.a.s_idx = s_idx;
						par_data->entity.ivar.var.a.e_idx = e_idx;
						par_data->entity.ivar.var.a.type = type;
						par_data->entity.ivar.bytes = getBytes(type);
					}
					else
					{
						par_data->entity.ivar.v_a = 0;
						par_data->entity.ivar.var.v.type = type;
						par_data->entity.ivar.bytes = getBytes(type);
					}		
					par_data->entity.ivar.code = id_num++;
					par_data->entity.ivar.scope = TREE->scope;
					par_data->entity.ivar.lno = token->lno;
					res = findScope(tableID,par_data->entity.ivar.word,TREE->scope,0);
					if(res.code >= 0)
					{
						if (print)
						{
							printf("\nERROR : Repeat declaration of identifier \"%s\" at line number %d",par_data->entity.ivar.word,par_data->entity.ivar.lno);
						}
	 					semantic_status = 0;
					}
					else
					{
						strcpy(par_data->entity.ivar.func_name, current_func);
						par_data->entity.ivar.depth = depth_in_scope;
						par_data->entity.ivar.offset = offset_in_function;
						addScope(tableID, par_data, 0);
						int bytes = par_data->entity.ivar.bytes;
						if (par_data->entity.ivar.v_a == 1) 
						{
							bytes *= (par_data->entity.ivar.var.a.e_idx - par_data->entity.ivar.var.a.s_idx + 1);
						}
						offset_in_function += bytes;
					}
					token = token->n;
				}
			}
		}
		else if(token->s == ID)
		{
			scopeChain* CURR = TREE;
			//traverse the symbol table tree for checking declaration of variable
			while(1)
			{
				res = findScope(tableID,token->val.s, CURR->scope, 0);
				if(res.code != -1)
				{
					break;
				}
				else
				{
					if(CURR->prev != NULL)
					{
						CURR = CURR->prev;
					}
					else
					{
						break;
					}
				}
			}		
			if(res.code == -1)
			{
				if (print)
				{
					printf("\nERROR : Undeclared variable \"%s\" at line number %d",token->val.s, token->lno);
				}
			 	semantic_status = 0;
			}
		}
		else if(token->s == DEF)
		{
			TREE->scope = ++univ_scope;
		}
		else if(token->s == DRIVERDEF)
		{
			TREE->scope = ++univ_scope;
			IDEntry* par_data = (IDEntry*)malloc(sizeof(IDEntry));
			strcpy(par_data->entity.ifunc.word, "program");
			par_data->entity.ifunc.status = defined;
			par_data->entity.ifunc.scope = TREE->scope;
			par_data->entity.ifunc.lno = token->lno;
			addScope(tableFunc,par_data, 1);
			strcpy(current_func,"program");
		}
		else if(token->s == SWITCH)
		{
			switch_scope_start = (token->lno) + 1;
			scopeChain* go = (scopeChain*)malloc(sizeof(scopeChain));
			go->scope = ++univ_scope;
			go->next = NULL;
			go->prev = TREE;
			go->in_cond = 0;
			go->in_loop = 0;
			TREE->next = go;
			TREE = TREE->next;
			assert(token->n != NULL);
			//ignore the opening bracket
			token = token->n->n;
			assert(token != NULL);
			go = TREE->prev;
			//check if switch case variable is declared or not
			while(1)
			{	
				res = findScope(tableID, token->val.s, go->scope, 0);
				if(res.code != -1)
				{
					break;
				}
				else
				{
					if(go->prev != NULL)
					{
						go = go->prev;
					}
					else
					{
						break;
					}
				}
			}
			if(res.code == -1)
			{
				if (print)
				{
					printf("\nERROR : Undeclared variable \"%s\" at line number %d",token->val.s, token->lno);
				}
			 	semantic_status = 0;
			}
			else
			{
				switch_type = res.node->entity.ivar.var.v.type;
				if(switch_type == REAL)
				{
					if (print)
					{
						printf("\nERROR : Line %d has a switch statement with an identifier of type real.", token->lno);
					}
			 		semantic_status = 0;
				}
				TREE->in_cond = 1;
			}
		}
		else if(token->s == CASE && TREE->in_cond == 1)
		{
			token = token->n;
			int case_type = token->s;
			if(switch_type == INTEGER && case_type != NUM)
			{
				switch_error[0] = 1;
			}
			else if(switch_type == BOOLEAN && case_type != TRUE_ && case_type != FALSE_)
			{
				switch_error[1]=1;
			}
		}
		else if(token->s == DEFAULT && TREE->in_cond == 1)
		{
			if(switch_type == BOOLEAN)
			{			
				switch_error[2]=1;
			}
			default_flag = 1;
		}
		else if(token->s == FOR || token->s == WHILE)
		{
			scopeChain* go = (scopeChain*)malloc(sizeof(scopeChain));
			go->scope = ++univ_scope;
			go->next = NULL;
			go->prev = TREE;
			go->in_cond = 0;
			go->in_loop = 1;
			TREE->next = go;
			TREE = TREE->next;
			if (token->s == FOR)
			{
				//ignore the brackets
				token = token->n->n;
				go = TREE->prev;
				assert(token != NULL);
				store2 ctrl_var; 
				while(1)
				{
					ctrl_var = findScope(tableID, token->val.s, go->scope, 0);
					if(ctrl_var.code != -1)
					{
						break;
					}
					else
					{
						if(go->prev != NULL)
						{
							go = go->prev;
						}
						else
						{
							break;
						}
					}
				}
				if (ctrl_var.code == -1)
				{
					printf("\nERROR : Line %d has \"FOR\" loop variable undefined\n", token->lno);
				}
				else 
				{
					checkFor(ctrl_var.node, token->prev->prev, tableID, print);
				}
			}
		}
		else if(token->s == START)
		{
			// printf("Entered Function : %s\n", current_func);
			scope_tot[TREE->scope].scope_start = token->lno;
			depth_in_scope += 1;
		}
		else if(token->s == END)
		{
			// printf("Exiting function : %s\n", current_func);
			scope_tot[TREE->scope].scope_end = token->lno;
			depth_in_scope -= 1;
			if((TREE->in_loop) == 1)
			{	
				TREE = TREE->prev;
				//move out of scope so remove the element from stack
				free(TREE->next);
			}
			else if((TREE->in_cond) == 1)
			{					
				switch_scope_end = token->lno;
				if(default_flag == 0 && switch_type != BOOLEAN)
				{
					if (print)
					{
						printf("\nERROR : The case statements lines <%d> to <%d> must have a default statement.", switch_scope_start, switch_scope_end);
					}
			 		semantic_status = 0;
				}
				default_flag = 0;
				if(switch_error[0] == 1)
				{
					if (print)
					{
						printf("\nERROR : The switch statement lines <%d>  to <%d> cannot have case statement with case keyword followed by any other value other than an integer ",switch_scope_start, switch_scope_end);
					}
			 		semantic_status = 0;
				}
				if(switch_error[1] == 1)
				{
					if (print)
					{
						printf("\nERROR : Lines <%d>to <%d> of the switch statement cannot have the case statements with integer labels.", switch_scope_start, switch_scope_end);
					}
			 		semantic_status = 0;
				}
				if(switch_error[2] == 1)
				{
					if (print)
					{
						printf("\nERROR : The switch statement lines <%d> to <%d> should not have a default statement. ",switch_scope_start, switch_scope_end);
					}
			 		semantic_status = 0;
				}
				int i;
				for(i = 0; i < 3; ++i)
				{
					switch_error[i] = -1;
				}
				TREE = TREE->prev;
				//move out of scope so remove the element from stack
				free(TREE->next);
			}
		}
		token = token->n;
	}
}

void secondRun(tokenInfo* token, hashTable2 tableID, hashTable2 tableFunc, int print)
{
	store2 res;
	scopeChain* TREE = (scopeChain*)malloc(sizeof(scopeChain));;
	TREE->scope = 0;
	TREE->next = NULL;
	TREE->prev = NULL;
	TREE->in_cond = 0;
	TREE->in_loop = 0;
	int univ_scope = 0;
	char current_func[10]; 
	strcpy(current_func," ");
	while(token != NULL)
	{
		if(token->s == MODULE)
		{
			if(token->prev->s == DEF)
			{
				strcpy(current_func,token->n->val.s);
			}
		}
		
		else if(token->s == DEF)
		{
			TREE->scope = ++univ_scope;
		}
		else if(token->s == DRIVERDEF)
		{
			TREE->scope = ++univ_scope;
			strcpy(current_func, "program");
		}
		else if(token->s == SWITCH)
		{
			scopeChain* go = (scopeChain*)malloc(sizeof(scopeChain));
			go->scope = ++univ_scope;
			go->next = NULL;
			go->prev = TREE;
			go->in_cond = 0;
			go->in_loop = 0;
			TREE->next = go;
			TREE = TREE->next;
			assert(token->n != NULL);
			//ignore the opening bracket
			token = token->n->n;
			assert(token != NULL);
			go = TREE->prev;
			//check if switch case variable is declared or not
			while(1)
			{	
				res = findScope(tableID, token->val.s, go->scope, 0);
				if(res.code != -1)
				{
					break;
				}
				else
				{
					if(go->prev != NULL)
					{
						go = go->prev;
					}
					else
					{
						break;
					}
				}
			}
			if (res.code != -1)
			{
				TREE->in_cond = 1;
			}
		}
		else if(token->s == END && (TREE->in_cond == 1 || TREE->in_loop == 1))
		{
			TREE = TREE->prev;
			free(TREE->next);
		}
		else if(token->s == FOR || token->s == WHILE)
		{
			scopeChain* go = (scopeChain*)malloc(sizeof(scopeChain));
			go->scope = ++univ_scope;
			go->next = NULL;
			go->prev = TREE;
			go->in_cond = 0;
			go->in_loop = 1;
			TREE->next = go;
			TREE = TREE->next;
		}
		else if(token->s == USE)
		{
			int flag;
			//ignore "module" lexeme
			token = token->n->n;
			res = findScope(tableFunc, token->val.s, 0, 1);
			if(strcmp(current_func, token->val.s) == 0)
			{
				//case of recursion, handled before
				while(token != NULL && token->s != SEMICOL)
				{
					token = token->n;
				}
			}			
			else if(res.code == -1)				
			{
				//case of undeclared function, handled before
				while(token != NULL && token->s != SEMICOL)
				{
					token = token->n;
				}
			}
			else
			{
				while(token->prev != NULL && token->prev->s != SEMICOL)
				{
					token = token->prev;
				}
				//check return type of function
				store2 actual_par;
				parameter* formal_par = res.node->entity.ifunc.outputList;
				if (formal_par == NULL)
				{
					if (token->s == SQBO)
					{
						if (print == 1)
						{
							printf("\nERROR : The function \"%s\" should  not return any value at line number %d",res.node->entity.ifunc.word,token->lno);
						}
						semantic_status = 0;
					}
				}
				else 
				{
					if(token->s != SQBO)
					{
						if (print == 1)
						{
							printf("\nERROR : The values returned by function \"%s\" are not stored at line number %d",res.node->entity.ifunc.word,token->lno);
						}
						semantic_status = 0;
					}
					else if(token->s == SQBO)
					{
						//ignore SQBO
						//checking of variables if declared has been done in previously
						token = token->n;
						scopeChain* go = TREE;
						while(1)
						{
							actual_par = findScope(tableID, token->val.s, go->scope,0);
							if(actual_par.code != -1)
							{
								break;
							}
							else
							{
								if (go->prev != NULL)
								{
									go = go->prev;
								}
								else
								{
									break;
								}
							}
						}
						flag = 0;
						while(formal_par != NULL)
						{
							if(actual_par.code != -1)
							{	
								if(actual_par.node->entity.ivar.var.v.type != formal_par->var.v.type)
								{
									if (print == 1)
									{
										printf("\nERROR : Type mismatch of output parameter \"%s\" at line number %d ",actual_par.node->entity.ivar.word, token->lno);
									}
									semantic_status = 0;
								}
							}
							formal_par = formal_par->next;
							//ignore comma
							token = token->n->n;
							if(token->prev->s != SQBC)
							{
								go = TREE;
								while(1)
								{	
									actual_par = findScope(tableID, token->val.s, go->scope,0);
									if(actual_par.code!=-1)
									{
										break;
									}
									else
									{
										if (go->prev != NULL)
										{
											go = go->prev;
										}
										else
										{
											break;
										}
									}
								}
							}
							else
							{	
								token = token->prev;
								flag = 1;
								break;
							}
						}
						if(flag == 1 && formal_par != NULL)
						{
							if (print == 1)
							{
								printf("\nERROR : The number of values returned from function \"%s\" are more than the number of accepting at line number %d",res.node->entity.ifunc.word,token->lno);
							}
							semantic_status = 0;
						}
						else if(flag == 0)
						{
							if (print == 1)
							{
								printf("\nERROR : The number of values returned from function \"%s\" are less than the number of accepting at line number %d",res.node->entity.ifunc.word,token->lno);
							}
							semantic_status = 0;
						}
					}
				}
				//check input type of function
				flag = 0;
				while(token != NULL && token->s != PARAMETERS)
				{
					token = token->n;
				}
				//ignore PARAMETERS
				token = token->n;
				formal_par = res.node->entity.ifunc.inputList;
				scopeChain* go = TREE;
				while(1)
				{
					actual_par = findScope(tableID, token->val.s, go->scope,0);
					if(actual_par.code != -1)
					{
						break;
					}
					else
					{
						if (go->prev != NULL)
						{
							go = go->prev;
						}
						else
						{
							break;
						}
					}
				}
				int flag = 0;
				while(formal_par != NULL)
				{
					if(actual_par.code != -1)
					{	
						if(actual_par.node->entity.ivar.v_a == 0)
						{
							if(formal_par->v_a != 0)
							{
								if (print == 1)
								{
									printf("\nERROR : Passed parameter \"%s\" at line number %d is a variable, whereas an array type was expected",actual_par.node->entity.ivar.word, token->lno);
								}
								semantic_status = 0;
							}
							else
							{
								if(actual_par.node->entity.ivar.var.v.type != formal_par->var.v.type)
								{
									if (print == 1)
									{
										printf("\nERROR : Type mismatch of input parameter \"%s\" at line number %d.", actual_par.node->entity.ivar.word, token->lno);
									}
									semantic_status = 0;
								}
							}	
						}
						else
						{
							if(formal_par->v_a != 1)
							{
								if (print == 1)
								{
									printf("\nPassed parameter \"%s\" at line number %d is an array, whereas a variable type was expected.",actual_par.node->entity.ivar.word, token->lno);
								}
								semantic_status = 0;
							}
							else
							{
								if(actual_par.node->entity.ivar.var.a.type != formal_par->var.a.type)
								{
									if (print == 1)
									{
										printf("\nERROR : Type mismatch of input parameter \"%s\" at line number %d.",actual_par.node->entity.ivar.word, token->lno);
									}
									semantic_status = 0;
								}
								if((actual_par.node->entity.ivar.var.a.s_idx != formal_par->var.a.s_idx) || (actual_par.node->entity.ivar.var.a.e_idx != formal_par->var.a.e_idx))
								{
									if (print == 1)
									{
										printf("\nERROR : Range mismatch of input parameter \"%s\" at line number %d.",actual_par.node->entity.ivar.word, token->lno);
									}
									semantic_status = 0;
								}
							}
						}
					}
					formal_par = formal_par->next;
					//ignore comma
					token = token->n->n;
					if(token->prev != NULL && token->prev->s != SEMICOL)
					{
						go = TREE;
						while(1)
						{
							actual_par = findScope(tableID, token->val.s, go->scope, 0);
							if(actual_par.code != -1)
							{
								break;
							}
							else
							{
								if(go->prev != NULL)
								{
									go=(go->prev);
								}
								else
								{
									break;
								}
							}
						}
					}
					else
					{	
						token = token->prev;
						flag = 1;
						break;
					}
				}
				if(flag == 1 && formal_par != NULL)
				{
					if (print == 1)
					{
						printf("\nERROR : The number of formal parameters of function \"%s\" are more than that of the actual parameters at line number %d",res.node->entity.ifunc.word,token->lno);
					}
					semantic_status = 0;
				}
				else if(flag == 0)
				{
					if (print == 1)
					{
						printf("\nERROR : The number of formal parameters of function \"%s\" are less than that of the actual parameters at line number %d",res.node->entity.ifunc.word,token->lno);
					}
					semantic_status = 0;					
				}
			}
		}
		token = token->n;
	}
}

void printVariables(hashTable2 table, totalScopeList *par) 
{
	int pos = 0;
	int SNO = 1;
	while(pos < hash_capacity_2)
	{
		linklist2 *root = table[pos].start;
		IDVariable go;
		if (table[pos].flag == occupied)
		{
			go = table[pos].ID->entity.ivar;
			if (go.v_a == 0)
			{
				printf("%4d %-10s\t%-20s %-10s %4d to %4d %6d %6d %6d\n", SNO, go.word, getLexeme(go.var.v.type), go.func_name, par[go.scope].scope_start, par[go.scope].scope_end, go.depth, go.bytes, go.offset);
				SNO += 1;
			}
			else
			{
				int range = (go.var.a.e_idx - go.var.a.s_idx + 1);
				int bytes = range * go.bytes;
				printf("%4d %-10s\tArray:%3d,%-10s %-10s %4d to %4d %6d %6d %6d\n", SNO, go.word, range, getLexeme(go.var.a.type), go.func_name, par[go.scope].scope_start, par[go.scope].scope_end, go.depth, bytes, go.offset);
				SNO += 1;
			}
			while(root != NULL)
			{
				go = root->ID->entity.ivar;
				if (go.v_a == 0)
				{
					printf("%4d %-10s\t%-20s %-10s %4d to %4d %6d %6d %6d\n", SNO, go.word, getLexeme(go.var.v.type), go.func_name, par[go.scope].scope_start, par[go.scope].scope_end, go.depth, go.bytes, go.offset);
					SNO += 1;
				}
				else
				{
					int range = (go.var.a.e_idx - go.var.a.s_idx + 1);
					int bytes = range * go.bytes;
					printf("%4d %-10s\tArray:%3d,%-10s %-10s %4d to %4d %6d %6d %6d\n", SNO, go.word, range, getLexeme(go.var.a.type), go.func_name, par[go.scope].scope_start, par[go.scope].scope_end, go.depth, bytes, go.offset);
					SNO += 1;
				}
				root = root->next;
			}
		}
		pos += 1;
	}
}

void printFunction(hashTable2 table, totalScopeList* par)
{
	int pos = 0;
	while(pos < hash_capacity_2)
	{
		linklist2* root = table[pos].start;
		IDFunction go;
		if (table[pos].flag == occupied)
		{
			go = table[pos].ID->entity.ifunc;
			printf("%-15s %10d %10d\n", go.word, par[go.scope].scope_start, par[go.scope].scope_end);
			while(root != NULL)
			{
				go = root->ID->entity.ifunc;
				printf("%-15s %10d %10d\n", go.word, par[go.scope].scope_start, par[go.scope].scope_end);
				root = root->next;
			}
		}
		pos += 1;
	}
}