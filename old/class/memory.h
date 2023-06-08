#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
class CheckMemory{
private:
	struct Node{
		void* ptr;
		Node* pnext;
		size_t len;
	};
	Node* memory;
	const char* error;
	FILE* fp;
	unsigned all;
	unsigned hashNum;
public:
	CheckMemory(unsigned hashNums=997)
	{
		if(hashNums<13)
			hashNum=hashNums;
		else
			hashNum=997;
#ifndef FILE_SAVE_NAME
		printf("recording...\n");
		fp=NULL;
#else
		fp=fopen(FILE_SAVE_NAME,"a+");
#endif
		memory=(Node*)malloc(sizeof(Node)*hashNum);
		error=NULL;
		all=0;
		if(memory==NULL)
		{
			error="malloc wrong";
			return;
		}
		memset(memory,0,sizeof(Node)*hashNum);
	}
	~CheckMemory()
	{
#ifndef FILE_SAVE_NAME
		printf("checking...\n");
#else
		if(fp!=NULL)
		{
			time_t now=time(NULL);
			fprintf(fp,"Date:%s\n",ctime(&now));
		}
#endif
		for(unsigned i=0;i<hashNum;i++)
		{
			Node* now=memory+i;
			now=now->pnext;
			while(now!=NULL)
			{
				all+=now->len;
#ifndef FILE_SAVE_NAME
				printf("leak %zu memory\n",now->len);
#else
				if(fp!=NULL)
					fprintf(fp,"leak:%zu\n",now->len);
#endif
				now=now->pnext;
			}
		}
#ifndef FILE_SAVE_NAME
		printf("all leck:%u\n\n",all);
#else
		if(fp!=NULL)
			fprintf(fp,"all leak:%u\n\n",all);
#endif
		if(memory!=NULL)
			free(memory);
		if(fp!=NULL)
			fclose(fp);
	}
	void insert(void* pmalloc,size_t size)
	{
		if(size==0||error!=NULL)
			return;
		long int temp=(long int)pmalloc;
		temp%=hashNum;
		Node* now=memory+temp;
		while(now->pnext!=NULL)
			now=now->pnext;
		now->pnext=(Node*)malloc(sizeof(Node));
		if(now->pnext==NULL)
		{
			error="malloc wrong";
			return;
		}
		now->len=size;
		now=now->pnext;
		now->len=size;
		now->pnext=NULL;
		now->ptr=pmalloc;
	}
	void change(void* pfree,void* newPtr,size_t newLen)
	{
		if(pfree==NULL||error!=NULL)
			return;
		long int temp=(long int)pfree;
		temp%=hashNum;
		Node* now=memory+temp,*last=now;
		if(now==NULL)
			return;
		now=now->pnext;
		while(now!=NULL)
		{
			if(now->ptr==pfree)
			{
				now->ptr=newPtr;
				now->len=newLen;
				break;
			}
			last=now;
			now=now->pnext;
		}
	}
	void erase(void* pfree)
	{
		if(pfree==NULL||error!=NULL)
			return;
		long int temp=(long int)pfree;
		temp%=hashNum;
		Node* now=memory+temp,*last=now;
		if(now==NULL)
			return;
		now=now->pnext;
		while(now!=NULL)
		{
			if(now->ptr==pfree)
			{
				last->pnext=now->pnext;
				free(now);
				now=NULL;
				break;
			}
			last=now;
			now=now->pnext;
		}
	}
}checkMemory;
void* operator new(size_t size)
{
	void* result=malloc(size);
	checkMemory.insert(result,size);
	return result;
}
void* operator new[](size_t size)
{
	void* result=malloc(size);
	checkMemory.insert(result,size);
	return result;
}
void operator delete(void* ptr) noexcept
{
	checkMemory.erase(ptr);
	free(ptr);
}
void operator delete[](void* ptr) noexcept
{
	if(ptr==NULL)
		return;
	checkMemory.erase(ptr);
	free(ptr);
}
void* Malloc_now(size_t size)
{
	void* temp=malloc(size);
	if(temp!=NULL)
		checkMemory.insert(temp,size);
	return temp;
}
inline void Free_now(void* ptr)
{
	checkMemory.erase(ptr);
	free(ptr);
}
void* Realloc_now(void* ptr,size_t size)
{
	void* temp=realloc(ptr,size);
	checkMemory.change(ptr,temp,size);
	return temp;
}
#define malloc(x) Malloc_now(x)
#define free(x) Free_now(x)
#define realloc(x,y) Realloc_now(x,y)
