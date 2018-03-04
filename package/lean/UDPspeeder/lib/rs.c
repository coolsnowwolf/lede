/*
 * rs.c
 *
 *  Created on: Sep 14, 2017
 *      Author: root
 */
#include "rs.h"
#include "stdlib.h"
#include "string.h"

void rs_encode(void *code,char *data[],int size)
{
	int k=get_k(code);
	int n=get_n(code);
	for(int i=k;i<n;i++)
	{
		fec_encode(code, (void **)data, data[i],i, size);
	}

	return ;
}

int rs_decode(void *code,char *data[],int size)
{
	int k=get_k(code);
	int n=get_n(code);
	int index[n];
	int count=0;
	for(int i=0;i<n;i++)
	{
		if(data[i]!=0)
		{
			index[count++]=i;
		}
	}
	if(count<k)
		return -1;
	for(int i=0;i<n;i++)
	{
		if(i<count)
			data[i]=data[index[i]];
		else
			data[i]=0;
	}
	return fec_decode(code,(void**)data,index,size);
}

static void * (*table)[256]=0;
void* get_code(int k,int n)
{
	if (table==0)
	{
		table=(void* (*)[256]) malloc(sizeof(void*)*256*256);
		if(!table)
		{
		    return table;
		}
		memset(table,0,sizeof(void*)*256*256);
	}
	if(table[k][n]==0)
	{
		table[k][n]=fec_new(k,n);
	}
	return table[k][n];
}
void rs_encode2(int k,int n,char *data[],int size)
{
	void* code=get_code(k,n);
	rs_encode(code,data,size);
}

int rs_decode2(int k,int n,char *data[],int size)
{
	void* code=get_code(k,n);
	return rs_decode(code,data,size);
}
