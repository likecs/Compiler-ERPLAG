/*
 *	COMPILER PROJECT- ERPLAG COMPILER
 *	Batch Number 74
 *	Bhuvnesh Jain : 2014A7PS028P
 *	Chirag Agarwal : 2014A7PS033P
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexerDef.h"

#define hash_capacity 31

hashTable initializeHashTable(int l)
{
	hashTable table;
	table = (hashTable)malloc(l * sizeof(entry));
	int i;
	for(i = 0; i < l; ++i)
	{
		table[i].code = -1;
		table[i].flag = available;
		table[i].start = (linklist *)malloc(sizeof(linklist));
	}
	return table;
}

int hashValue(char *key)
{
	int len = strlen(key);
	int i, value = 0, p = 1, prime = 5;
	//Rolling Hash on strings
	for(i = len - 1; i >= 0; --i)
	{
		value = (value + p * (key[i]-'0')) % hash_capacity;
		if (value < 0)
		{
			value += hash_capacity;
		}
		p = (p * prime) % hash_capacity;
	}
	return value;
}

store findKeyword(hashTable T, char *searchKey)
{
	store res;
	int pos = hashValue(searchKey);
	if(T[pos].flag == available)
	{
		res.code = -1;
		strcpy(res.word, "\0");
	}
	else if(strcmp(T[pos].word, searchKey) == 0)
	{
		res.code = T[pos].code;
		strcpy(res.word, searchKey);
	}
	else
	{
		linklist* root = (linklist *)malloc(sizeof(linklist));
		root = T[pos].start;
		while (root != NULL)
		{
			if(strcmp(root->word, searchKey) == 0)
			{	
				res.code = root->code;
				strcpy(res.word, searchKey);
				return res; 
			}
			root = root->next;
		}
		res.code = -1;
	}
	return res;
}

void addKeyword(hashTable T, int c, char *key)
{   
	int pos = hashValue(key);
	if(T[pos].flag == available || T[pos].flag == deleted)
	{
		T[pos].code = c;
		strcpy(T[pos].word,key);
		T[pos].flag = occupied;
		T[pos].start = NULL;
	}
	else 
	{
		linklist* root = (linklist *)malloc(sizeof(linklist));
		strcpy(root->word, key);
		root->code = c;
		root->next = NULL;
		linklist* temp = T[pos].start;		
		if(temp == NULL)
		{
			T[pos].start = root;
		}
		else 
		{
			while(temp->next != NULL)
			{	
				temp = temp->next;
			}
			temp->next = root;
		}
	}
}
