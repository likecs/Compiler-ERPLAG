/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#include "lexer.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "symbolDef.h"
#include "symbolTable.h"

#include "ast.h"
#include "assembler.h"

int rop = 0;
int nol = -1;
int for_loop = 0;
int while_loop = 0;
int switch_loop = 0;
int inp_var = -1, out_var = -1;

void WriteData(hashTable2 h, FILE * fp) 
{
	int pos = 0;
	while (pos < hash_capacity_2) 
	{
		linklist2* temp = h[pos].start;
		
		if (h[pos].flag == occupied) 
		{
			if (h[pos].ID->entity.ivar.v_a == 0) 
			{
				if (h[pos].ID->entity.ivar.var.v.type == REAL)
				{
					fprintf(fp, "%s%d_cb: RESD 1\n", h[pos].ID->entity.ivar.word, h[pos].ID->entity.ivar.scope);
				}
				else if (h[pos].ID->entity.ivar.var.v.type == INTEGER)
					fprintf(fp, "%s%d_cb: RESW 1\n", h[pos].ID->entity.ivar.word, h[pos].ID->entity.ivar.scope);
				else
					fprintf(fp, "%s%d_cb: RESB 1\n", h[pos].ID->entity.ivar.word, h[pos].ID->entity.ivar.scope);
			} 
			else 
			{
				if (h[pos].ID->entity.ivar.var.a.type == REAL)
					fprintf(fp, "%s%d_cb: RESD %d\n", h[pos].ID->entity.ivar.word, h[pos].ID->entity.ivar.scope, (h[pos].ID->entity.ivar.var.a.e_idx - h[pos].ID->entity.ivar.var.a.s_idx + 1));
				else if (h[pos].ID->entity.ivar.var.a.type == INTEGER)
					fprintf(fp, "%s%d_cb: RESW %d\n", h[pos].ID->entity.ivar.word, h[pos].ID->entity.ivar.scope, (h[pos].ID->entity.ivar.var.a.e_idx - h[pos].ID->entity.ivar.var.a.s_idx + 1));
				else
					fprintf(fp, "%s%d_cb: RESB %d\n", h[pos].ID->entity.ivar.word, h[pos].ID->entity.ivar.scope, (h[pos].ID->entity.ivar.var.a.e_idx - h[pos].ID->entity.ivar.var.a.s_idx + 1));
			}
			while (temp != NULL) 
			{
				if (temp->ID->entity.ivar.v_a == 0) 
				{
					if (temp->ID->entity.ivar.var.v.type == REAL)
						fprintf(fp, "%s%d_cb: RESD 1\n", temp->ID->entity.ivar.word, temp->ID->entity.ivar.scope);
					else if (temp->ID->entity.ivar.var.v.type == INTEGER)
						fprintf(fp, "%s%d_cb: RESW 1\n", temp->ID->entity.ivar.word, temp->ID->entity.ivar.scope);
					else
						fprintf(fp, "%s%d_cb: RESB 1\n", temp->ID->entity.ivar.word, temp->ID->entity.ivar.scope);
				} 
				else 
				{
					if (temp->ID->entity.ivar.var.a.type == REAL)
						fprintf(fp, "%s%d_cb: RESD %d\n", temp->ID->entity.ivar.word, temp->ID->entity.ivar.scope, (temp->ID->entity.ivar.var.a.e_idx - temp->ID->entity.ivar.var.a.s_idx + 1));
					else if (temp->ID->entity.ivar.var.a.type == INTEGER)
						fprintf(fp, "%s%d_cb: RESW %d\n", temp->ID->entity.ivar.word, temp->ID->entity.ivar.scope, (temp->ID->entity.ivar.var.a.e_idx - temp->ID->entity.ivar.var.a.s_idx + 1));
					else
						fprintf(fp, "%s%d_cb: RESB %d\n", temp->ID->entity.ivar.word, temp->ID->entity.ivar.scope, (temp->ID->entity.ivar.var.a.e_idx - temp->ID->entity.ivar.var.a.s_idx + 1));
				}
				temp = temp->next;
			}
		}
		pos++;
	}
}

void writeNode(ASTNode a,FILE *fp) 
{
	int Vi;
	float Vf;
	char * value;
	char * in ;
	if (a->t == non_terminal) 
	{
		if (a->e.nt == var) 
		{
			if (a->childQ.front->e.t == ID)
				if (a->childQ.front->next == NULL) 
				{
					value = a->childQ.front->token->val.s;
					fprintf(fp, "MOV AX, [%s%d_cb]\n", value, a->childQ.front->link->entity.ivar.scope);
				} 
				else 
				{
					value = a->childQ.front->token->val.s; in = (char * ) a->childQ.front->next->childQ.front->token->val.s;
					fprintf(fp, "MOV BL, [%s%d_cb]\nSUB BL,1\nMOV AL, 2\nMUL BL\nMOV BX, AX\nMOV AX, [%s%d_cb+EBX]\n", in , a->childQ.front->next->childQ.front->link->entity.ivar.scope, value, a->childQ.front->link->entity.ivar.scope);
				} 
				else if (a->childQ.front->e.t == NUM) 
				{
					Vi = a->childQ.front->token->val.i;
					fprintf(fp, "MOV AX, %d\n", Vi);
				}
			 	else 
			 	{
					Vf = a->childQ.front->token->val.f;
					fprintf(fp, "MOV AX, %d\n", (int)Vf);
				}
			ASTNode p;
			p = a->parent;
			if (p->t == non_terminal) 
			{
				if (p->e.nt == factor || p->e.nt == arithmeticExpr || p->e.nt == term_1 || p->e.nt == AnyTerm || a->e.nt == arithmeticOrBooleanExpr)
				{
					nol++;
					fprintf(fp, "MOV [lft+%d],AX\n", nol * 2);
				}
				else if (p->e.nt == N4 || p->e.nt == N5 || p->e.nt == N7 || p->e.nt == N8)
					fprintf(fp, "MOV [rgt], AX\n");

			}
			if (p->t == terminal) 
			{ 
				if (p->e.nt != ASSIGNOP)
				{
					if (p->childQ.front == a) 
					{
						nol++;
						fprintf(fp, "MOV [lft+%d],AX\n", nol * 2);
					} 
					else
						fprintf(fp, "MOV [rgt], AX\n");
				}

			} 
		}
			else if (!(a->childQ.front->t == non_terminal && a->childQ.front->e.nt == var) && (a->e.nt == factor || a->e.nt == arithmeticExpr || a->e.nt == arithmeticOrBooleanExpr || a->e.nt == term_1 || a->e.nt == AnyTerm))
			{
				ASTNode p;
				p = a->parent;
				if (p->t == terminal) 
				{ 
					if (p->e.t != ASSIGNOP)
						if (p->childQ.front == a) 
						{
							nol++;
							fprintf(fp, "MOV [lft+%d],AX\n", nol * 2);
						} 
						else
							fprintf(fp, "MOV [rgt], AX\n");
				} 
				else 
				{ 
					if ((p->e.nt == N4 || p->e.nt == N5 || p->e.nt == N7 || p->e.nt == N8) && p->e.nt != iterativeStmt)
						fprintf(fp, "MOV [rgt], AX\n");
				}
			} 
			else if (((a->childQ.front->t == terminal) && (isArithmeticOperator(a->childQ.front->e.t))) && (a->e.nt == N4 || a->e.nt == N5 || a->e.nt == N7 || a->e.nt == N8))
				fprintf(fp, "MOV [rgt], AX\n");
		
		else if (a->e.nt == expression) 
		{
			if (a->childQ.front->t == terminal) 
			{
				if (a->childQ.front->e.t == MINUS)
					fprintf(fp, "MOV AX, -1\nIMUL word [temp]\n");
			}
		}
	} 
	else 
	{
		if (a->e.t == PLUS && a->parent->e.nt != expression) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nADD AX,[rgt]\n", nol * 2);
			nol--;
		} 
		else if (a->e.t == MINUS && a->parent->e.nt != expression) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nSUB AX,[rgt]\n", nol * 2);
			nol--;
		}
		else if (a->e.t == MUL) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nIMUL word [rgt]\n", nol * 2);
			nol--;
		} 
		else if (a->e.t == DIV) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nCWD\nIDIV [rgt]\n", nol * 2);
			nol--;
		} 
		else if (a->e.t == GT) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nCMP AX,[rgt]\nJG TRUE%d\nMOV AX,0\nJMP CONT%d\nTRUE%d: MOV AX,1\nCONT%d:", nol * 2, rop, rop, rop, rop);
			rop++;
			nol--;
		} 
		else if (a->e.t == GE) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nCMP AX,[rgt]\nJGE TRUE%d\nMOV AX,0\nJMP CONT%d\nTRUE%d: MOV AX,1\nCONT%d:", nol * 2, rop, rop, rop, rop);
			rop++;
			nol--;
		} 
		else if (a->e.t == LT) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nCMP AX,[rgt]\nJL TRUE%d\nMOV AX,0\nJMP CONT%d\nTRUE%d: MOV AX,1\nCONT%d:", nol * 2, rop, rop, rop, rop);
			rop++;
			nol--;
		} 
		else if (a->e.t == LE) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nCMP AX,[rgt]\nJLE TRUE%d\nMOV AX,0\nJMP CONT%d\nTRUE%d: MOV AX,1\nCONT%d:", nol * 2, rop, rop, rop, rop);
			rop++;
			nol--;
		} 
		else if (a->e.t == EQ) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nCMP AX,[rgt]\nJE TRUE%d\nMOV AX,0\nJMP CONT%d\nTRUE%d: MOV AX,1\nCONT%d:", nol * 2, rop, rop, rop, rop);
			rop++;
			nol--;
		} 
		else if (a->e.t == NE) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nCMP AX,[rgt]\nJNE TRUE%d\nMOV AX,0\nJMP CONT%d\nTRUE%d: MOV AX,1\nCONT%d:", nol * 2, rop, rop, rop, rop);
			rop++;
			nol--;
		} 
		else if (a->e.t == AND) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nAND AX,[rgt]\n", nol * 2);
			nol--;
		} 
		else if (a->e.t == OR) 
		{
			fprintf(fp, "MOV AX,[lft+%d]\nOR AX,[rgt]\n", nol * 2);
			nol--;
		} 
		else if (a->e.t == ASSIGNOP) 
		{
			if (a->childQ.size == 2) 
			{
				value = a->childQ.front->token->val.s;
				fprintf(fp, "MOV [%s%d_cb], AX\n", value, a->childQ.front->link->entity.ivar.scope);
			} 
			else 
			{
				value = a->childQ.front->token->val.s;
				if (a->childQ.front->next->childQ.front->e.t == NUM) 
				{
					Vi = a->childQ.front->next->childQ.front->token->val.i;
					fprintf(fp, "MOV [%s%d_cb+%d], AX\n", value, a->childQ.front->link->entity.ivar.scope, (Vi - 1) * 2);
				}
				else
				{
					 in = (char * ) a->childQ.front->next->childQ.front->token->val.s;
					 fprintf(fp, "MOV [temp], AX\nMOV AL, 2\nMOV BL, [%s%d_cb]\nSUB BL,1\nMUL BL\nMOV BX, AX\nMOV AX, [temp]\nMOV [%s%d_cb+EBX],AX\n", in , a->childQ.front->next->childQ.front->link->entity.ivar.scope, value, a->childQ.front->link->entity.ivar.scope);
				}
			}
		}
	}
}





void parseQueue(ASTNode a, FILE * fp)
{
	ASTNode c;
	if (a->t == non_terminal)
	{
		if (a->e.nt == assignmentStmt || a->e.nt == whichStmt || a->e.nt == lvalueIDStmt || a->e.nt == lvalueARRStmt || a->e.nt == index_nt || a->e.nt == expression || a->e.nt == arithmeticOrBooleanExpr || a->e.nt == N7 || a->e.nt == AnyTerm || a->e.nt == N8 || a->e.nt == arithmeticExpr || a->e.nt == N4 || a->e.nt == N5 || a->e.nt == factor || a->e.nt == term_1) 
		{
			c = a->childQ.front;
			while (c != NULL) 
			{
				parseQueue(c,fp);
				c = c->next;
			}
			writeNode(a,fp);
		}
		else if (a->e.nt == var)
		{
			writeNode(a,fp);
		}		
		else if (a->e.nt == iterativeStmt || a->e.nt == conditionalStmt) 
		{
			c = a->childQ.front;
			parseQueue(c,fp);
		}
		else if (a->e.nt == ioStmt) 
		{
			c = a->childQ.front;
			parseQueue(c,fp);
		}
		else 
		{
			writeNode(a,fp);
			c = a->childQ.front;
			while (c != NULL)
			{
				parseQueue(c,fp);
				c = c->next;
			}
		}
	}	 
	else if (a->t == terminal) 
	{
		if (isArithmeticOperator(a->e.t) || isLogicalOperator(a->e.t) || isRelationalOperator(a->e.t) || a->e.t == ASSIGNOP) 
		{
			c = a->childQ.front;
			while (c != NULL) 
			{
				parseQueue(c,fp);
				c = c->next;
			}
			writeNode(a,fp);
		} 
		else if (a->e.t == FOR) 
		{
			int sc; //scope info of variable
			a = a->next;
			char s[10];
			strcpy(s, a->link->entity.ivar.word);
			sc = a->link->entity.ivar.scope;
			a = a->next->childQ.front;
			int num1 = (int) a->token->val.i;
			fprintf(fp, "MOV word [%s%d_cb], %d\n", s, sc, num1);
			a = a->next;
			int num2 = (int) a->token->val.i;
			fprintf(fp, "MOV CX, %d\n", num2);
			int this_for = for_loop;
			fprintf(fp, "MOV [FOR_CTRL+%d], CX\n", this_for * 2);
			fprintf(fp, "FOR%d: ", for_loop++);
			a = a->parent->next;
			parseQueue(a,fp);
			fprintf(fp, "INC word [%s%d_cb]\n", s, sc);
			fprintf(fp, "MOV CX,[FOR_CTRL+%d]\n", this_for * 2);
			fprintf(fp, "CMP CX, [%s%d_cb]\n", s, sc);
			fprintf(fp, "JGE FOR%d\n", this_for);
		}
		else if (a->e.t == WHILE) 
		{
			a = a->next;
			int this_while = while_loop;
			fprintf(fp, "WHILE%d: ", while_loop++);
			parseQueue(a,fp);
			fprintf(fp, "MOV CX, AX\n");
			fprintf(fp, "CMP CX, 1\n");
			fprintf(fp, "JNZ END_WHILE%d\n", this_while);
			a = a->next;	
			parseQueue(a,fp);
			fprintf(fp, "JMP WHILE%d\n", this_while);
			fprintf(fp, "END_WHILE%d: ", this_while);
		}
		else if (a->e.t == SWITCH) 
		{
			int sc;
			a = a->next;
			char s[10];
			strcpy(s, a->link->entity.ivar.word);
			sc = a->link->entity.ivar.scope;
			fprintf(fp, "MOV CX, [%s%d_cb]\n", s, sc);
			int this_switch = switch_loop;
			switch_loop++;
			int case_number = 2;
			term sm;
			c = a->next->next;
			if (a->next->e.nt == caseStmts || a->next->e.nt == N9) 
			{
				a = a->next->childQ.front;
				sm = a->childQ.front->token->s;
				if (sm == TRUE_)
					fprintf(fp, "CMP CX, 1\n");
				else if (sm == FALSE_)
					fprintf(fp, "CMP CX, 0\n");
				else
					fprintf(fp, "CMP CX, %d\n", a->childQ.front->token->val.i);
			}
			while (a->next->next->e.nt == caseStmts || a->next->next->e.nt == N9) 
			{
				fprintf(fp, "JNZ CASE%d%d\n", this_switch, case_number);
				a = a->next;
				parseQueue(a,fp);
				fprintf(fp, "JMP END_SWITCH%d\n", this_switch);
				fprintf(fp, "CASE%d%d: ", this_switch, case_number++);
				a = a->next->childQ.front;
				sm = a->childQ.front->token->s;
				if (sm == TRUE_)
					fprintf(fp, "CMP CX, 1\n");
				else if (sm == FALSE_)
					fprintf(fp, "CMP CX, 0\n");
				else
					fprintf(fp, "CMP CX, %d\n", a->childQ.front->token->val.i);
				if (a->next->next == NULL)
					break;
			}
			fprintf(fp, "JNZ DEFAULT%d\n", this_switch);
			a = a->next;
			parseQueue(a,fp);
			fprintf(fp, "JMP END_SWITCH%d\n", this_switch);
			//printf("\nc:%d\n",c->e.nt);
			if (c != NULL) 
			{
				fprintf(fp, "DEFAULT%d: ", this_switch);
				parseQueue(c,fp);
			}
			else
			{
				fprintf(fp, "DEFAULT%d: \n", this_switch);
			}
			fprintf(fp, "END_SWITCH%d: ", this_switch);
		} 
		else if (a->e.t == GET_VALUE) 
		{
			inp_var++;
			char s[10];
			a = a->next;
			strcpy(s, a->link->entity.ivar.word);
			int lscope = a->link->entity.ivar.scope;
			// fprintf(fp, "MOV %s%d_cb,0\nLOOP_IN%d: XOR AX, AX\nMOV AH,1H\nINT 21H\nCMP AL,0AH\nJZ NEXT_IN%d\nSUB AL,30H\nMOV DL, AL\nMOV AL, %s%d_cb\nMOV BL, 10\nMUL BL\nADD AX, DX\nMOV %s%d_cb, AX\nJMP LOOP_IN%d\n NEXT_IN%d:", s, a->link->entity.ivar.scope, inp_var, inp_var, s, a->link->entity.ivar.scope, s, a->link->entity.ivar.scope, inp_var, inp_var);
			if(a->link->entity.ivar.v_a == 0) 
			{
				fprintf(fp, "call readInt\nMOV [%s%d_cb], AX\n", s,lscope);
			}
			else
			{
				int asize = a->link->entity.ivar.var.a.e_idx - a->link->entity.ivar.var.a.s_idx +1;
				fprintf(fp, "MOV ESI, 0\n");
				int this_for = for_loop;
				fprintf(fp, "FOR%d: ", for_loop++);
				fprintf(fp, "call readInt\nMOV [%s%d_cb + ESI], AX\n", s,lscope);
				fprintf(fp, "INC ESI\n");
				fprintf(fp, "CMP ESI, %d\n", asize);
				fprintf(fp, "JL FOR%d\n", this_for);
			}
		} 
		else if (a->e.t == PRINT) 
		{
			out_var++;
			char s[10];
			a = a->next->childQ.front;
			if (a->e.t == NUM) 
			{
				fprintf(fp, "MOV AX, %d\ncall printInt\n",a->token->val.i);
			} 
			else
			{
				int sc;
				char s[12];
				strcpy(s, a->link->entity.ivar.word);
				sc = a->link->entity.ivar.scope;
				if (a->next != NULL) 
				{
					a = a->next->childQ.front;
					char s2[12]; //
					strcpy(s2, a->link->entity.ivar.word);
					fprintf(fp, "MOV SI, [%s%d_cb]\n", s2, a->link->entity.ivar.scope);
					fprintf(fp, "MOV AX, [%s%d_cb+SI]\ncall printInt\n", s, sc);
				
				} 
				else 
				{
					fprintf(fp, "MOV AX, [%s%d_cb]\ncall printInt\n",s, sc);
				}
			}
		}
	}
	return;
}