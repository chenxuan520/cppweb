#include<stdio.h>
#include<pthread.h>
int addNum(int a,int b)
{
	pthread_create(NULL,NULL,NULL,NULL);
	return a+b;
}
