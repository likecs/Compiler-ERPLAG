#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "ast.h"
#include "astDef.h"
#include "parser.h"
#include "symbolTable.h"

int scopeF = 0;
int scopeOther = 0;
int maxScope = 0;
int inOther = 0;
int ERROR_IN_TYPE = 0;
int ERROR_IN_LINE = 0;
int scope_temp;
int N_Declare_Error = 0;
int total_size;

// check whether to remove the terminal from PARSE Tree or not, to get the AST
int isValidTerminal(term t)
{
	if (t == PRINT || t == GET_VALUE || t == ARRAY || t == FOR || t == WHILE || t == SWITCH) 
	{
		return 1;
	}
	else if (t == TRUE_ || t == FALSE_ || t == MINUS || t == BOOLEAN || t == INTEGER) 
	{
		return 1;
	}
	return 0;
}

int isLogicalOperator(term op) 
{
	if (op == OR || op == AND)
	{
		return 1;
	}
	return 0;
}

int isArithmeticOperator(term op)
{
	if (op == PLUS || op == MINUS || op == MUL || op == DIV)
	{
		return 1;
	}
	return 0;
}

int isRelationalOperator(term op) 
{
	if(op == LT || op == NE || op == GE || op == GT || op == EQ || op == LE)
	{
		return 1;
	}
	return 0;
}

ASTNode createASTNode(ASTNode parent, gElement e, tag t, tokenInfo* tokenLink, IDEntry* link, valASTNode val) 
{
	ASTNode go;
	go = (ASTNode)malloc(sizeof(struct ASTNode));
	go->scope = -1;
	go->t = t;
	go->link = link;
	go->next = NULL;
	go->e = e;
	go->tokenLink = tokenLink;
	go->val = val;
	go->parent = parent;
	go->childQ.front = go->childQ.back = NULL;
	go->childQ.size = 0;
	return go;
}

ASTree buildASTree(parseTree PT, hashTable2 tableID, hashTable2 tableFunc, int print)
{
	PT.c_node = PT.root;
	ASTree _ast;
	_ast.root = NULL;
	_ast.currentNode = _ast.root;
	_ast.root = AddToQueueAST(PT.root, _ast.root, NULL, tableID, tableFunc);
	_ast.currentNode = _ast.root;
	printASTree(_ast, print);
	return _ast;
}

ASTNode FillNodeAST(pNode p, ASTNode parent, hashTable2 tableID, hashTable2 tableFunc)
{
	ASTNode temp = NULL;
	valASTNode v;
	store2 f;
	if (p->t == non_terminal) 
	{
		if (p->e.nt != op1 && p->e.nt != op2 && p->e.nt != logicalOp && p->e.nt != relationalOp) 
		{
			temp = createASTNode(parent, p->e, p->t, p->token_link, NULL, v);
		}
		else 
		{ 
			temp = NULL;
		}
	}
	else 
	{
		if (p->e.t == ID) 
		{
			if (p->parent->e.nt == moduleDeclaration || p->parent->e.nt == module || p->parent->e.nt == moduleReuseStmt)
			{
				f = findScope(tableFunc, p->token_link->val.s, scopeF, 1);
				scope_temp = scopeF;
				while (f.code == -1 && scope_temp != 0) 
				{
					scope_temp -= 1;
					f = findScope(tableFunc, p->token_link->val.s, scope_temp, 1);
				}
				temp = createASTNode(parent, p->e, p->t, p->token_link, f.node, v);
				temp->scope = scopeF;
			}
			else  
			{
				if (inOther == 0)
				{
					f = findScope(tableID, p->token_link->val.s, scopeF, 0);
					scope_temp = scopeF;
				}
				else
				{
					f = findScope(tableID, p->token_link->val.s, scopeOther, 0);
					scope_temp = scopeOther;
				}
				while (f.code == -1 && scope_temp != 0)
				{
					scope_temp -= 1;
					f = findScope(tableID, p->token_link->val.s, scope_temp, 0);
				}
				if (f.code != -1)
				{
					//declared variable (Type is known)
					temp = createASTNode(parent, p->e, p->t, p->token_link, f.node, v);
					temp->scope = scope_temp;
					if (f.node->entity.ivar.v_a == 0)
					{
						temp->datatype = f.node->entity.ivar.var.v.type;
					}
					else
					{
						temp->datatype = f.node->entity.ivar.var.a.type;
					}
				} 
				else 
				{
					//undeclared variable case
					temp = createASTNode(parent, p->e, p->t, p->token_link, NULL, v);
				}
			} 
		} 
		else if (p->e.t == NUM) 
		{
			v.num = p->token_link->val.i;
			temp = createASTNode(parent, p->e, p->t, p->token_link, NULL, v);
			temp->datatype = (term)INTEGER;
		} 
		else if (p->e.t == RNUM) 
		{
			v.rnum = p->token_link->val.f;
			temp = createASTNode(parent, p->e, p->t, p->token_link, NULL, v);
			temp->datatype = (term)REAL;
		} 
		else if (isValidTerminal(p->e.t) == 1) 
		{
			temp = createASTNode(parent, p->e, p->t, p->token_link, NULL, v);
			if (p->e.t == (term)TRUE_ || p->e.t == (term)FALSE_)
			{
				temp->datatype = (term)BOOLEAN;
			}
		}
	} 
	return temp;
}

ASTNode AddToQueueAST(pNode p, ASTNode a, ASTNode parent,hashTable2 tableID, hashTable2 tableFunc) 
{
	pNode t;
	if (p->t == non_terminal && (p->e.nt == iterativeStmt || p->e.nt == conditionalStmt)) 
	{
		inOther++;
		maxScope++;
		scopeOther = maxScope;
	} else if (p->t == non_terminal && (p->e.nt == moduleDeclaration || p->e.nt == module || p->e.nt == driverModule)) {
		t = (pNode) malloc(sizeof(pNode));
		maxScope++;
		scopeF = maxScope;
	}
	a = FillNodeAST(p, parent,  tableID, tableFunc);
	valASTNode v;
	if (p->t == non_terminal)
	{
		if (p->e.nt == assignmentStmt)
		{
			ASTNode tbs, lchild, rchild;
			//tbs=FillNodeAssignOP(p,temp,NULL,tableID,tableFunc) 
			tbs = createASTNode(a, (gElement)(term) ASSIGNOP, (tag) terminal, p->child.front->token_link, NULL, v);
			lchild = FillNodeAST(p->child.front, tbs, tableID, tableFunc); 
			pNode p1, p2;
			//<tbsmentStmt> = ID <whichStmt>
			p1 = p->child.front->next;
			//<whichStmt> = <lvalueIDStmt> || <lvalueARRStmt>
			if (p1->child.front->e.nt == lvalueARRStmt) 
			{
				p2 = p1->child.front->child.front->next; 
				ASTNode ID;  
				//<index_nt>  //in our changed grammar this is ID
				ID = AddToQueueAST(p2, ID, tbs,  tableID, tableFunc);
				//expression
				rchild = AddToQueueAST(p2->next->next->next, rchild, ID, tableID, tableFunc);
				tbs->childQ = enqueueAST(tbs->childQ, lchild);
				tbs->childQ = enqueueAST(tbs->childQ, ID);
				//
				if (rchild->childQ.size == 1 && rchild->childQ.front->t != terminal) 
				{
					rchild->childQ.front->parent = tbs;
					tbs->childQ = enqueueAST(tbs->childQ, rchild->childQ.front); 
					free(rchild);
				} 
				else 
				{
					rchild->parent = tbs;
					tbs->childQ = enqueueAST(tbs->childQ, rchild);
				}
				a->childQ = enqueueAST(a->childQ, tbs);
			} 
			else 
			{
				rchild = AddToQueueAST(p1, rchild, tbs, tableID, tableFunc);
				tbs->childQ = enqueueAST(tbs->childQ, lchild);
				if (rchild->childQ.size == 1 && rchild->childQ.front->t != terminal)
				{
					rchild->childQ.front->parent = tbs;
					tbs->childQ = enqueueAST(tbs->childQ, rchild->childQ.front);
					free(rchild);
				} 
				else 
				{
					rchild->parent = tbs;
					tbs->childQ = enqueueAST(tbs->childQ, rchild);
				}
				a->childQ = enqueueAST(a->childQ, tbs);
			}
		} 
		else if (p->e.nt == arithmeticOrBooleanExpr) 
		{
			// arithmaticOrBoolean->anyterm,N7, where N7!=e
			ASTNode lchild, rchild, op;
			lchild = AddToQueueAST(p->child.front, lchild, a,  tableID, tableFunc);
			if (lchild->t == non_terminal && lchild->e.nt == AnyTerm) 
			{
				pNode k;
				k = p->child.front->next;
				rchild = AddToQueueAST(k, rchild, a,  tableID, tableFunc);
				if (k->child.front->t == non_terminal)
				{
					op = createASTNode(a, k->child.front->child.front->e, k->child.front->child.front->t, k->child.front->child.front->token_link, NULL, v);
					if (lchild->childQ.size == 1 && lchild->childQ.front->t != terminal) 
					{
						lchild->childQ.front->parent = op; 
						op->childQ = enqueueAST(op->childQ, lchild->childQ.front);
						free(lchild);
					} 
					else {
						lchild->parent = op;
						op->childQ = enqueueAST(op->childQ, lchild);
					}

					if (rchild->childQ.size == 1 && rchild->childQ.front->t != terminal) {
						rchild->childQ.front->parent = op;
						op->childQ = enqueueAST(op->childQ, rchild->childQ.front);
						free(rchild);
					} 
					else {
						rchild->parent = op;
						op->childQ = enqueueAST(op->childQ, rchild);
					}
					op->parent = a;
					a->childQ = enqueueAST(a->childQ, op);
				} 
				else 
				{
					if (lchild->childQ.size == 1 && lchild->childQ.front->t != terminal) {
						lchild->childQ.front->parent = a;
						a->childQ = enqueueAST(a->childQ, lchild->childQ.front);
						free(lchild);
					} 
					else {
						lchild->parent = a;
						a->childQ = enqueueAST(a->childQ, lchild);
					}
				}
			} 
			else 
			{
				if (lchild->childQ.size == 1 && lchild->childQ.front->t != terminal) 
				{
					lchild->childQ.front->parent = a;
					a->childQ = enqueueAST(a->childQ, lchild->childQ.front);
					free(lchild);
				} 
				else 
				{
					lchild->parent = a;
					a->childQ = enqueueAST(a->childQ, lchild); 
				}
			}
		} 
		else if (p->e.nt == N7 || p->e.nt == N8 || p->e.nt == N4 || p->e.nt == N5) 
		{
			//// Nx =   <op><a><Nx> | e
			if (p->child.front->t != terminal)
			{
				if (p->child.front->next->next->child.front->t == terminal) 
				{
					t = p->child.front->next;
					while (t != NULL) 
					{
						ASTNode child;
						child = AddToQueueAST(t, child, parent, tableID, tableFunc);
						if ((child != NULL) && (!(child->t == non_terminal && child->childQ.front == NULL)))
						{
							if (child->childQ.size == 1 && child->childQ.front->t != terminal) 
							{
								child->childQ.front->parent = a;
								a->childQ = enqueueAST(a->childQ, child->childQ.front);
								free(child);
							} 
							else 
							{
								child->parent = a;
								a->childQ = enqueueAST(a->childQ, child);
							}
						}
						if (t->t == non_terminal && (t->e.nt == iterativeStmt || t->e.nt == conditionalStmt)) 
						{
							inOther--;
							scopeOther--;
						}
						t = t->next;
						if (child != NULL)
							child = child->next;
					}
				} 
				else 
				{
					ASTNode lchild, rchild, op;
					op = createASTNode(a, p->child.front->next->next->child.front->child.front->e, 
										p->child.front->next->next->child.front->child.front->t, 
										p->child.front->next->next->child.front->child.front->token_link,
										NULL, v);
					lchild = AddToQueueAST(p->child.front->next, lchild, op, tableID, tableFunc);
					rchild = AddToQueueAST(p->child.front->next->next, rchild, op, tableID, tableFunc);         
					if (lchild->childQ.size == 1 && lchild->childQ.front->t != terminal) 
					{
						lchild->childQ.front->parent = op;
						op->childQ = enqueueAST(op->childQ, lchild->childQ.front);
						free(lchild);
					} 
					else 
					{
						lchild->parent = op;
						op->childQ = enqueueAST(op->childQ, lchild);
					}
					if (rchild->childQ.size == 1 && rchild->childQ.front->t != terminal) 
					{
						rchild->childQ.front->parent = op;
						op->childQ = enqueueAST(op->childQ, rchild->childQ.front);
						free(rchild);
					} 
					else 
					{
						rchild->parent = op;
						op->childQ = enqueueAST(op->childQ, rchild);
					}
					op->parent = a;
					a->childQ = enqueueAST(a->childQ, op);
				} 
			}
			else 
			{
				t = p->child.front;
				while (t != NULL) 
				{
					ASTNode child;
					child = AddToQueueAST(t, child, parent, tableID, tableFunc);
					if ((child != NULL) && (!(child->t == non_terminal && child->childQ.front == NULL))) 
					{
						if (child->childQ.size == 1 && child->childQ.front->t != terminal) 
						{
							child->childQ.front->parent = a;
							a->childQ = enqueueAST(a->childQ, child->childQ.front);
							free(child);
						} 
						else 
						{
							child->parent = a;
							a->childQ = enqueueAST(a->childQ, child);
						}
					}
					if (t->t == non_terminal && (t->e.nt == iterativeStmt || t->e.nt == conditionalStmt)) 
					{
						inOther--;
						scopeOther--;
					}
					t = t->next;
					if (child != NULL)
						child = child->next;
				}
			}
		} 
		else if (p->e.nt == AnyTerm || p->e.nt == arithmeticExpr || p->e.nt == term_1) //DONE correct
		{
			pNode temp = p->child.front->next ;
			if (temp->child.front->t == non_terminal) 
			{
				ASTNode lchild, rchild, op;           
				op = createASTNode(a, temp->child.front->child.front->e,    
															temp->child.front->child.front->t, 
															temp->child.front->child.front->token_link, NULL, v);
				lchild = AddToQueueAST(p->child.front, lchild, op,  tableID, tableFunc);
				rchild = AddToQueueAST(temp, rchild, op,  tableID, tableFunc);
				lchild->parent = op;
				rchild->parent = op;
				op->childQ = enqueueAST(op->childQ, lchild);
				op->childQ = enqueueAST(op->childQ, rchild);
				op->parent = a; 
				a->childQ = enqueueAST(a->childQ, op);
			} 
			else 
			{
				t = p->child.front;
				while (t != NULL) 
				{
					ASTNode child;
					child = AddToQueueAST(t, child, parent, tableID, tableFunc);
					if ((child != NULL) && (!(child->t == non_terminal && child->childQ.front == NULL))) 
					{
						if (child->childQ.size == 1 && child->childQ.front->t != terminal) 
						{
							child->childQ.front->parent = a;
							a->childQ = enqueueAST(a->childQ, child->childQ.front);
							free(child);
						} 
						else 
						{
							child->parent = a;
							a->childQ = enqueueAST(a->childQ, child);
						}
					}
					if (t->t == non_terminal && (t->e.nt == iterativeStmt || t->e.nt == conditionalStmt)) 
					{
						inOther--;
						scopeOther--;
					}
					t = t->next;
					if (child != NULL)
						child = child->next;
				}
			}
		} 
		else //for all other casess
		{
			if (a != NULL) 
			{
				t = p->child.front;
			} 
			else t = NULL;

			while (t != NULL) 
			{
				ASTNode child;
				child = AddToQueueAST(t, child, parent, tableID, tableFunc);
				if ((child != NULL) && (!(child->t == non_terminal && child->childQ.front == NULL))) 
				{
					//reducing depth if child is a non terminal 
					if (child->childQ.size == 1 && child->childQ.front->t != terminal) 
					{
						child->childQ.front->parent = a;
						a->childQ = enqueueAST(a->childQ, child->childQ.front);
						free(child);
					} 
					else 
					{
						child->parent = a;
						a->childQ = enqueueAST(a->childQ, child);
					}
				}
				if (t->t == non_terminal && (t->e.nt == iterativeStmt || t->e.nt == conditionalStmt)) 
				{
					inOther--;
					scopeOther--;
				}
				t = t->next;
				if (child != NULL)
				{
					child = child->next;
				}
			}
		}
	} 
	return a;
}

ASTQueue enqueueAST(ASTQueue q, ASTNode p) 
{
	if (q.size != 0) 
	{
		q.back->next = p;
		q.back = p;
		if (q.front->next == NULL)
		{
			q.front->next = q.back;
		}
	}
	else
	{
		q.front = p;
		q.back = q.front;
	}
	q.size += 1;
	return q;
}

int ASTQueueTypeChecker(ASTNode a, int print) 
{
	ASTNode c;
	c = a->childQ.front;
	while (c != NULL && (N_Declare_Error == 0)) 
	{
		ASTQueueTypeChecker(c, print);
		c = c->next;
	}
	a = typeAssignerAndChecker(a, print);
	return ERROR_IN_TYPE;
}

ASTNode typeAssignerAndChecker(ASTNode a, int print) 
{
	if (a->t == terminal) 
	{
		if (isArithmeticOperator(a->e.t)) 
		{
			if ((a->e.t == MINUS || a->e.t == PLUS) && a->parent->e.nt == expression)
			{
				if (a->next->datatype == INTEGER || a->next->datatype == REAL) 
				{
					a->datatype = a->next->datatype;
				} 
				else 
				{
					//this error gets detected in second pass
					if (ERROR_IN_LINE == 0 && print == 1)
					{
						printf("\nERROR : Unary Minus or plus can't be assigned to boolean at line %d.", a->tokenLink->lno);
					}
					ERROR_IN_TYPE = 1;
					ERROR_IN_LINE = 1;
				} 
			}
			else
			{
				if ((a->childQ.front->datatype == a->childQ.front->next->datatype) && (a->childQ.front->datatype == INTEGER || a->childQ.front->datatype == REAL)) 
				{
					a->datatype = a->childQ.front->datatype;
				}
				else 
				{
					if (ERROR_IN_LINE == 0 && print == 1) 
					{
						printf("\nERROR : Only same datatypes (integer or real) can be added/subtracted/multiplied/divided at line at line %d.", a->tokenLink->lno);
					}
					ERROR_IN_TYPE = 1;
					ERROR_IN_LINE = 1;
				}
			}
		} 
		else if (isRelationalOperator(a->e.t))
		{
			if ((a->childQ.front->datatype == a->childQ.front->next->datatype) && (a->childQ.front->datatype == INTEGER || a->childQ.front->datatype == REAL)) 
			{
				a->datatype = (term) BOOLEAN;
			} 
			else 
			{
				if (ERROR_IN_LINE == 0 && print == 1) 
				{
					printf("\nERROR : Only same datatypes (integer or real) can be compared at line %d.", a->tokenLink->lno);
				}
				ERROR_IN_TYPE = 1;
				ERROR_IN_LINE = 1;
			} 
		}
		else if (isLogicalOperator(a->e.t))
		{
			if ((a->childQ.front->datatype == a->childQ.front->next->datatype) && (a->childQ.front->datatype == BOOLEAN)) 
			{
				a->datatype = (term)BOOLEAN;
			} 
			else 
			{
				if (ERROR_IN_LINE == 0 && print == 1) 
				{
					printf("\nERROR : Only boolean datatypes can be operated with AND/OR at line %d.", a->tokenLink->lno);
				}
				ERROR_IN_TYPE = 1;
				ERROR_IN_LINE = 1;
			} 
		}
		else if ((a->e.t == ASSIGNOP) && (a->childQ.front->datatype != a->childQ.back->datatype)) 
		{
			assert(a->parent->e.nt == assignmentStmt);
			if (ERROR_IN_LINE == 0 && print == 1)
			{
				printf("\nERROR : Left and right hand side of assignment operator are not same at line %d.", a->tokenLink->lno);
			}
			ERROR_IN_TYPE = 1;
			ERROR_IN_LINE = 0;
		} 
	} 
	else
	{
		if (a->e.nt != program) 
		{
			if ((a->e.nt == statements || a->e.nt == statement || a->e.nt == ioStmt || a->e.nt == iterativeStmt || a->e.nt == simpleStmt || a->e.nt == declareStmt || a->e.nt == conditionalStmt) || 
				(a->parent->e.nt == iterativeStmt && (a->e.nt == expression || a->e.nt == arithmeticOrBooleanExpr || a->e.nt == AnyTerm || a->e.nt == arithmeticExpr || a->e.nt == factor || a->e.nt == term_1)))
			{
				N_Declare_Error = 0;
				ERROR_IN_LINE = 0;
			}
			if (a->e.nt == whichStmt || a->e.nt == lvalueIDStmt || a->e.nt == lvalueARRStmt || a->e.nt == index_nt || a->e.nt == expression || a->e.nt == arithmeticOrBooleanExpr || a->e.nt == AnyTerm 
				|| a->e.nt == arithmeticExpr || a->e.nt == term_1 || a->e.nt == factor || a->e.nt == var  || a->e.nt == whichID  || a->e.nt == N4  || a->e.nt == N5 || a->e.nt == N7  || a->e.nt == N8 )
			{  
				a->datatype = a->childQ.front->datatype;
			}
			if (((a->e.nt == expression || a->e.nt == var) && a->parent->e.nt == iterativeStmt) && (a->datatype != BOOLEAN)) 
			{
				if (ERROR_IN_LINE == 0 && print == 1) 
				{
					printf("\nERROR : WHILE condition can only contain BOOLEAN expressions at line %d", a->childQ.front->tokenLink->lno);
				}
				ERROR_IN_TYPE = 1;
			}
			if ((a->e.nt == whichID) && (a->datatype != INTEGER)) 
			{
				if (ERROR_IN_LINE == 0 && print == 1)
				{
					printf("\nERROR : Index of Array should be INTEGER at line %d", a->childQ.front->tokenLink->lno);
				}
				ERROR_IN_TYPE = 1;
			}
		}
	}
	return a;	
}

void printASTree(ASTree a, int print)
{
	no_of_ast_tree_nodes = 0;
	a.currentNode = a.root;
	if (print == 1)
	{
		printf("%-25s:%-10s:%-25s:%-12s", "CurrentNode", "Value", "ParentNode", "Line number");
	}
	printASTreeQueue(a.root, print);
}

void printASTreeQueue(ASTNode a, int print)
{
	while (a != NULL) 
	{
		printASTNode(a, print);
		if (a->childQ.front != NULL)
		{
			printASTreeQueue(a->childQ.front, print);
		}
		a = a->next;
	}
}

void printASTNode(ASTNode a, int print)
{
	no_of_ast_tree_nodes += 1;
	if (print == 0)
	{
		return ;
	}
	char *parent, *s, *datatype, empty[10], n[10], no[10];
	strcpy(empty, "e");
	strcpy(n, "---");
	strcpy(no, "-n");
	if (a->parent != NULL)
	{
		if (a->parent->t == terminal)
		{
			parent = getToken(a->parent->e.t);
		}
		else
		{
			parent = getNonTerm(a->parent->e.nt);
		}
	}
	else 
	{
		parent = (char * )NULL;
	}
	if (a->t == terminal)
	{
		s = getToken(a->e.t);
		if (a->e.t != e)
		{ 
			if (a->e.t == NUM)
			{
				printf("\n%-25s:%-10d:%-25s:%-12d", s, a->val.num, parent, a->tokenLink->lno);
			}
			else if (a->e.t == RNUM)
			{
				printf("\n%-25s:%-10f:%-25s:%-12d", s, a->val.rnum, parent, a->tokenLink->lno);
			}
			else if (a->e.t == ID)
			{
				printf("\n%-25s:%-10s:%-25s:%-12d", s, n, parent, a->tokenLink->lno);
			}
			else 
			{
				printf("\n%-25s:%-10s:%-25s:%-12d", s, n, parent, a->tokenLink->lno);
			}
		} 
		else 
		{
			printf("\n%-25s:%-10s:%-25s:%-12s", empty, n, parent, no);
		}
	}   
	else
	{
		s = getNonTerm(a->e.nt);
		printf("\n%-25s:%-10s:%-25s:%-12s", s, n, parent, no);
	}
}