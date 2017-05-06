/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbolDef.h"

hashTable2 initializeHashTable2(int l)
{
	hashTable2 table;
	table = (hashTable2)malloc(l * sizeof(entry2));
	int i;
	for(i = 0; i < l; ++i)
	{
		table[i].flag = available;
		table[i].start = NULL;
		table[i].ID = NULL;
	}
	return(table);
}

int hashValue2(char *key)
{
	int len = strlen(key);
	int i, value = 0, p = 1, prime = 5;
	//Rolling Hash on strings
	for(i = len - 1; i >= 0; --i)
	{
		value = (value + p * (key[i]-'0')) % hash_capacity_2;
		if (value < 0)
		{
			value += hash_capacity_2;
		}
		p = (p * prime) % hash_capacity_2;
	}
	return value;
}

store2 findScope(hashTable2 T, char *searchKey, int scope, int option)
{	
	store2 res;
	int pos = hashValue2(searchKey);
	if(T[pos].flag == available)
	{
		res.code = -1;
	}
	else if(option == 0)
	{
		if(strcmp(T[pos].ID->entity.ivar.word,searchKey) == 0)
		{
			if(scope == T[pos].ID->entity.ivar.scope)
			{
				res.code = 1;
				res.node = T[pos].ID;
			}
			else
			{
				linklist2* root = (linklist2 *)malloc(sizeof(linklist2));
				root = T[pos].start;
				while (root != NULL)
				{
					if(strcmp(root->ID->entity.ivar.word, searchKey) == 0)
					{
						if(scope == root->ID->entity.ivar.scope)
						{	
							res.code = 1;
							res.node = (root->ID);
							return res;
						}
					}
					root = root->next;
				}
				res.code = -1;
			}
		}
		else
		{
			linklist2* root = (linklist2 *)malloc(sizeof(linklist2));
			root = T[pos].start;
			while (root != NULL)
			{
				if(strcmp(root->ID->entity.ivar.word, searchKey) == 0)
				{	
					if(scope == root->ID->entity.ivar.scope)
					{	
						res.code = 1;
						res.node = (root->ID);
						return res;
					}
				}
				root = root->next;
			}
			res.code = -1;
		}
	}
	else
	{
		if(strcmp(T[pos].ID->entity.ifunc.word,searchKey) == 0)
		{
			res.node = (T[pos].ID);
			res.code = 1;
		}
		else
		{
			linklist2* root = (linklist2 *)malloc(sizeof(linklist2));
			root = T[pos].start;
			while (root != NULL)
			{
				if(strcmp(root->ID->entity.ifunc.word, searchKey) == 0)
				{
					res.code = 1;
					res.node = (root->ID);
					return res;
				}
				root = root->next;
			}
			res.code = -1;
		}
	}
	return res;
}

void addScope(hashTable2 T, IDEntry* M, int option)
{
	int pos;
	if(option == 0)   
	{
		pos = hashValue2(M->entity.ivar.word);
	}
	else
	{
		pos = hashValue2(M->entity.ifunc.word);
	}
	if(T[pos].flag == available || T[pos].flag == deleted)
	{
		T[pos].ID = M;
		T[pos].flag = occupied;
		T[pos].start = NULL;
	}
	else 
	{
		linklist2* root = (linklist2 *)malloc(sizeof(linklist2));
		root->ID = M;   
		root->next = NULL;
		linklist2 *temp = T[pos].start;
		if(temp == NULL)		
		{
			T[pos].start = root;
		}
		else
		{
			while (temp->next != NULL)
			{
				temp = temp->next;
			}
			temp->next = root;
		}
	}
}