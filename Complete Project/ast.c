/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "ast.h"
#include "astDef.h"
#include "parser.h"
#include "symbolTable.h"
#include "assembler.h"

int scope_in_module = 0;
int in_other = 0;
int scope_other = 0;

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

ASTNode createASTNode(ASTNode parent, gElement e, tag t, tokenInfo* token, IDEntry* link) 
{
	ASTNode go = (ASTNode)malloc(sizeof(struct ASTNode));
	go->t = t;
	go->e = e;
	go->scope = -1;
	go->link = link;
	go->token = token;
	go->next = NULL;
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
	_ast.root = fillASTQueue(PT.root, _ast.root, NULL, tableID, tableFunc);
	_ast.currentNode = _ast.root;
	printASTree(_ast, print);
	return _ast;
}

ASTNode fillASTNode(pNode p, ASTNode parent, hashTable2 tableID, hashTable2 tableFunc)
{
	store2 f;
	int scope_temp;
	ASTNode temp = NULL;
	if (p->t == non_terminal) 
	{
		if (p->e.nt != op1 && p->e.nt != op2 && p->e.nt != logicalOp && p->e.nt != relationalOp) 
		{
			temp = createASTNode(parent, p->e, p->t, p->token_link, NULL);
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
				f = findScope(tableFunc, p->token_link->val.s, scope_in_module, 1);
				scope_temp = scope_in_module;
				while (f.code == -1 && scope_temp != 0) 
				{
					scope_temp -= 1;
					f = findScope(tableFunc, p->token_link->val.s, scope_temp, 1);
				}
				temp = createASTNode(parent, p->e, p->t, p->token_link, f.node);
				temp->scope = scope_in_module;
			}
			else  
			{
				if (in_other == 0)
				{
					f = findScope(tableID, p->token_link->val.s, scope_in_module, 0);
					scope_temp = scope_in_module;
				}
				else
				{
					f = findScope(tableID, p->token_link->val.s, scope_other, 0);
					scope_temp = scope_other;
				}
				while (f.code == -1 && scope_temp != 0)
				{
					scope_temp -= 1;
					f = findScope(tableID, p->token_link->val.s, scope_temp, 0);
				}
				if (f.code != -1)
				{
					//declared variable (Type is known)
					temp = createASTNode(parent, p->e, p->t, p->token_link, f.node);
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
					temp = createASTNode(parent, p->e, p->t, p->token_link, NULL);
				}
			} 
		} 
		else if (p->e.t == NUM) 
		{
			temp = createASTNode(parent, p->e, p->t, p->token_link, NULL);
			temp->datatype = (term)INTEGER;
		} 
		else if (p->e.t == RNUM) 
		{
			temp = createASTNode(parent, p->e, p->t, p->token_link, NULL);
			temp->datatype = (term)REAL;
		} 
		else if (isValidTerminal(p->e.t) == 1) 
		{
			temp = createASTNode(parent, p->e, p->t, p->token_link, NULL);
			if (p->e.t == (term)TRUE_ || p->e.t == (term)FALSE_)
			{
				temp->datatype = (term)BOOLEAN;
			}
		}
	} 
	return temp;
}

ASTNode fillASTQueue(pNode p, ASTNode a, ASTNode parent,hashTable2 tableID, hashTable2 tableFunc) 
{
	pNode t = (pNode) malloc(sizeof(pNode));
	if (p->t == non_terminal && (p->e.nt == iterativeStmt || p->e.nt == conditionalStmt)) 
	{
		in_other += 1;
		scope_in_module += 1;
		scope_other = scope_in_module;
	} 
	else if (p->t == non_terminal && (p->e.nt == moduleDeclaration || p->e.nt == module || p->e.nt == driverModule)) 
	{
		scope_in_module += 1;
	}
	a = fillASTNode(p, parent,  tableID, tableFunc);
	if (p->t == non_terminal)
	{
		if (p->e.nt == assignmentStmt)
		{
			pNode p1, p2;
			ASTNode operator, left_child, right_child;
			operator = createASTNode(a, (gElement)(term) ASSIGNOP, (tag) terminal, p->child.front->token_link, NULL);
			left_child = fillASTNode(p->child.front, operator, tableID, tableFunc); 
			p1 = p->child.front->next;
			if (p1->child.front->e.nt == lvalueARRStmt) 
			{
				p2 = p1->child.front->child.front->next; 
				ASTNode ID;  
				ID = fillASTQueue(p2, ID, operator,  tableID, tableFunc);
				right_child = fillASTQueue(p2->next->next->next, right_child, ID, tableID, tableFunc);
				operator->childQ = enqueueAST(operator->childQ, left_child);
				operator->childQ = enqueueAST(operator->childQ, ID);
			} 
			else 
			{
				right_child = fillASTQueue(p1, right_child, operator, tableID, tableFunc);
				operator->childQ = enqueueAST(operator->childQ, left_child);
			}
			if (right_child->childQ.size != 1 || right_child->childQ.front->t == terminal)
			{
				right_child->parent = operator;
				operator->childQ = enqueueAST(operator->childQ, right_child);
			} 
			else 
			{
				right_child->childQ.front->parent = operator;
				operator->childQ = enqueueAST(operator->childQ, right_child->childQ.front);
				free(right_child);
			}
			a->childQ = enqueueAST(a->childQ, operator);
		} 
		else if (p->e.nt == arithmeticOrBooleanExpr) 
		{
			ASTNode left_child, right_child, op;
			left_child = fillASTQueue(p->child.front, left_child, a,  tableID, tableFunc);
			if (left_child->t == non_terminal && left_child->e.nt == AnyTerm) 
			{
				pNode k;
				k = p->child.front->next;
				right_child = fillASTQueue(k, right_child, a,  tableID, tableFunc);
				if (k->child.front->t == non_terminal)
				{
					op = createASTNode(a, k->child.front->child.front->e, k->child.front->child.front->t, k->child.front->child.front->token_link, NULL);
					if (left_child->childQ.size == 1 && left_child->childQ.front->t != terminal) 
					{
						left_child->childQ.front->parent = op; 
						op->childQ = enqueueAST(op->childQ, left_child->childQ.front);
						free(left_child);
					} 
					else 
					{
						left_child->parent = op;
						op->childQ = enqueueAST(op->childQ, left_child);
					}
					if (right_child->childQ.size == 1 && right_child->childQ.front->t != terminal) 
					{
						right_child->childQ.front->parent = op;
						op->childQ = enqueueAST(op->childQ, right_child->childQ.front);
						free(right_child);
					} 
					else 
					{
						right_child->parent = op;
						op->childQ = enqueueAST(op->childQ, right_child);
					}
					op->parent = a;
					a->childQ = enqueueAST(a->childQ, op);
				} 
				else 
				{
					if (left_child->childQ.size != 1 || left_child->childQ.front->t == terminal) 
					{
						left_child->parent = a;
						a->childQ = enqueueAST(a->childQ, left_child);
					} 
					else 
					{
						left_child->childQ.front->parent = a;
						a->childQ = enqueueAST(a->childQ, left_child->childQ.front);
						free(left_child);
					}
				}
			}
			else 
			{
				if (left_child->childQ.size != 1 || left_child->childQ.front->t == terminal) 
				{
					left_child->parent = a;
					a->childQ = enqueueAST(a->childQ, left_child); 
				} 
				else 
				{
					left_child->childQ.front->parent = a;
					a->childQ = enqueueAST(a->childQ, left_child->childQ.front);
					free(left_child);
				}
			}
		} 
		else if (p->e.nt == N4 || p->e.nt == N5 || p->e.nt == N7 || p->e.nt == N8) 
		{
			if (p->child.front->t != terminal)
			{
				if (p->child.front->next->next->child.front->t == terminal) 
				{
					t = p->child.front->next;
					while (t != NULL) 
					{
						ASTNode child;
						child = fillASTQueue(t, child, parent, tableID, tableFunc);
						if ((child != NULL) && (child->t == terminal || child->childQ.front != NULL))
						{
							if (child->childQ.size != 1 || child->childQ.front->t == terminal) 
							{
								child->parent = a;
								a->childQ = enqueueAST(a->childQ, child);
							}
							else 
							{
								child->childQ.front->parent = a;
								a->childQ = enqueueAST(a->childQ, child->childQ.front);
								free(child);
							}
						}
						if (t->t == non_terminal && (t->e.nt == iterativeStmt || t->e.nt == conditionalStmt)) 
						{
							in_other--;
							scope_other--;
						}
						if (child != NULL)
						{
							child = child->next;
						}
						t = t->next;
					}
				} 
				else
				{
					ASTNode left_child, right_child, op;
					op = createASTNode(a, p->child.front->next->next->child.front->child.front->e, 
										p->child.front->next->next->child.front->child.front->t, 
										p->child.front->next->next->child.front->child.front->token_link, NULL);
					left_child = fillASTQueue(p->child.front->next, left_child, op, tableID, tableFunc);
					right_child = fillASTQueue(p->child.front->next->next, right_child, op, tableID, tableFunc);         
					if (left_child->childQ.size != 1 || left_child->childQ.front->t == terminal) 
					{
						left_child->parent = op;
						op->childQ = enqueueAST(op->childQ, left_child);
					} 
					else 
					{
						left_child->childQ.front->parent = op;
						op->childQ = enqueueAST(op->childQ, left_child->childQ.front);
						free(left_child);
					}
					if (right_child->childQ.size != 1 || right_child->childQ.front->t == terminal) 
					{
						right_child->parent = op;
						op->childQ = enqueueAST(op->childQ, right_child);
					} 
					else 
					{
						right_child->childQ.front->parent = op;
						op->childQ = enqueueAST(op->childQ, right_child->childQ.front);
						free(right_child);
					}
					assert(op->parent == a);
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
					child = fillASTQueue(t, child, parent, tableID, tableFunc);
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
						in_other--;
						scope_other--;
					}
					t = t->next;
					if (child != NULL)
					{
						child = child->next;
					}
				}
			}
		} 
		else if (p->e.nt == AnyTerm || p->e.nt == arithmeticExpr || p->e.nt == term_1) 
		{
			pNode temp = p->child.front->next ;
			if (temp->child.front->t == non_terminal) 
			{
				ASTNode left_child, right_child, op;           
				op = createASTNode(a, temp->child.front->child.front->e, temp->child.front->child.front->t, 
									temp->child.front->child.front->token_link, NULL);
				left_child = fillASTQueue(p->child.front, left_child, op,  tableID, tableFunc);
				right_child = fillASTQueue(temp, right_child, op,  tableID, tableFunc);
				left_child->parent = op;
				right_child->parent = op;
				op->childQ = enqueueAST(op->childQ, left_child);
				op->childQ = enqueueAST(op->childQ, right_child);
				op->parent = a; 
				a->childQ = enqueueAST(a->childQ, op);
			} 
			else 
			{
				t = p->child.front;
				while (t != NULL) 
				{
					ASTNode child;
					child = fillASTQueue(t, child, parent, tableID, tableFunc);
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
						in_other--;
						scope_other--;
					}
					t = t->next;
					if (child != NULL)
					{
						child = child->next;
					}
				}
			}
		}
		else
		{
			if (a != NULL) 
			{
				t = p->child.front;
			} 
			else t = NULL;

			while (t != NULL) 
			{
				ASTNode child;
				child = fillASTQueue(t, child, parent, tableID, tableFunc);
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
					in_other--;
					scope_other--;
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

int error_type = 0;
int error_line = 0;
int not_declare_error = 0;

int ASTQueueTypeChecker(ASTNode a, int print) 
{
	ASTNode c;
	c = a->childQ.front;
	while (c != NULL && (not_declare_error == 0)) 
	{
		ASTQueueTypeChecker(c, print);
		c = c->next;
	}
	a = typeAssignerAndChecker(a, print);
	return error_type;
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
					if (error_line == 0 && print == 1)
					{
						printf("\nERROR : Unary Minus or plus can't be assigned to boolean at line %d.", a->token->lno);
					}
					error_type = 1;
					error_line = 1;
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
					if (error_line == 0 && print == 1) 
					{
						printf("\nERROR : Only same datatypes (integer or real) can be added/subtracted/multiplied/divided at line %d.", a->token->lno);
					}
					error_type = 1;
					error_line = 1;
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
				if (error_line == 0 && print == 1) 
				{
					printf("\nERROR : Only same datatypes (integer or real) can be compared at line %d.", a->token->lno);
				}
				error_type = 1;
				error_line = 1;
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
				if (error_line == 0 && print == 1) 
				{
					printf("\nERROR : Only boolean datatypes can be operated with AND/OR at line %d.", a->token->lno);
				}
				error_type = 1;
				error_line = 1;
			} 
		}
		else if ((a->e.t == ASSIGNOP) && (a->childQ.front->datatype != a->childQ.back->datatype)) 
		{
			assert(a->parent->e.nt == assignmentStmt);
			if (error_line == 0 && print == 1)
			{
				printf("\nERROR : Left and right hand side of assignment operator are not same at line %d.", a->token->lno);
			}
			error_type = 1;
			error_line = 0;
		} 
	} 
	else
	{
		if (a->e.nt != program) 
		{
			if (a->e.nt == statements || a->e.nt == statement || a->e.nt == ioStmt || a->e.nt == iterativeStmt || a->e.nt == simpleStmt || a->e.nt == declareStmt || a->e.nt == conditionalStmt)
			{
				not_declare_error = 0;
				error_line = 0;
			}
			if (a->e.nt == whichStmt || a->e.nt == lvalueIDStmt || a->e.nt == lvalueARRStmt || a->e.nt == index_nt || a->e.nt == expression || a->e.nt == arithmeticOrBooleanExpr || a->e.nt == AnyTerm 
				|| a->e.nt == arithmeticExpr || a->e.nt == term_1 || a->e.nt == factor || a->e.nt == var  || a->e.nt == whichID  || a->e.nt == N4  || a->e.nt == N5 || a->e.nt == N7  || a->e.nt == N8 )
			{  
				a->datatype = a->childQ.front->datatype;
			}
			if (((a->e.nt == expression || a->e.nt == var) && a->parent->e.nt == iterativeStmt) && (a->datatype != BOOLEAN)) 
			{
				if (error_line == 0 && print == 1) 
				{
					printf("\nERROR : WHILE condition can only contain BOOLEAN expressions at line %d", a->childQ.front->token->lno);
				}
				error_type = 1;
			}
			if ((a->e.nt == whichID) && (a->datatype != INTEGER)) 
			{
				if (error_line == 0 && print == 1)
				{
					printf("\nERROR : Index of Array should be INTEGER at line %d", a->childQ.front->token->lno);
				}
				error_type = 1;
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
				printf("\n%-25s:%-10d:%-25s:%-12d", s, a->token->val.i, parent, a->token->lno);
			}
			else if (a->e.t == RNUM)
			{
				printf("\n%-25s:%-10f:%-25s:%-12d", s, a->token->val.f, parent, a->token->lno);
			}
			else if (a->e.t == ID)
			{
				printf("\n%-25s:%-10s:%-25s:%-12d", s, n, parent, a->token->lno);
			}
			else 
			{
				printf("\n%-25s:%-10s:%-25s:%-12d", s, n, parent, a->token->lno);
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