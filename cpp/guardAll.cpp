#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
/********************************
	author:chenxuan
	date:2021/9/29
	funtion:enhance class for guard can guard many things at the same time
*********************************/
class GuardProcess{
private:
	int pid;
public:
	GuardProcess(bool isAllHide=false)
	{
		pid=0;
		pid=fork();
		if(pid!=0)
			exit(0);
		if(true==isAllHide)
		{
			pid=setsid();
			if(-1==pid)
				throw 1;
			pid=chdir("/");
			if(-1==pid)
				throw 2;
			umask(0);
			close(STDERR_FILENO);
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
		}
	}
	void working(int (*pfuncWork)(void*),void* arg)
	{
		while(pfuncWork(arg)==0);	
	}
	static bool changeWorkingDir(const char* dirPath)
	{
		int temp=chdir(dirPath);
		if(-1==temp)
			return false;
		return true;
	}
	static inline void showLastError()
	{
		perror("last error");
	}
};
/********************************
	author:chenxuan
	date:2021/9/25
	funtion:file class to do something
*********************************/
class FileGet{
private:
	char* pbuffer;
public:
	FileGet()
	{
		pbuffer=NULL;
	}
	~FileGet()
	{
		if(pbuffer!=NULL)
		{
			free(pbuffer);
			pbuffer=NULL;
		}
	}
	int getFileLen(const char* fileName)
	{
		int len=0;
		FILE* fp=fopen(fileName,"rb");
		if(fp==NULL)
			return -1;
		fseek(fp,0,SEEK_END);
		len=ftell(fp);
		fclose(fp);
		return len;
	}
	bool getFileMsg(const char* fileName,char* buffer)
	{
		int i=0,len=0;
		len=this->getFileLen(fileName);
		FILE* fp=fopen(fileName,"rb");
		if(fp==NULL)
			return false;
		for(i=0;i<len;i++)
			buffer[i]=fgetc(fp);
		buffer[i+1]=0;
		fclose(fp);
		return true;
	}
	bool fileStrstr(const char* fileName,const char* strFind)
	{
		int len=0;
		char* pstr=NULL;
		len=this->getFileLen(fileName);
		if(pbuffer!=NULL)
		{
			free(pbuffer);
			pbuffer=NULL;
		}
		FILE* fp=fopen(fileName,"r");
		if(fp==NULL)
			return false;
		pbuffer=(char*)malloc(sizeof(char)*(len+10));
		char* ptemp=pbuffer;
		if(pbuffer==NULL)
			return false;
		memset(pbuffer,0,sizeof(char)*(len+5));
		if(false==this->getFileMsg(fileName,pbuffer))
			return false;
		while((*ptemp<65||*ptemp>122)&&ptemp<pbuffer+sizeof(char)*len)
			ptemp++;
		pstr=strstr(ptemp,strFind);
		if(pbuffer!=NULL)
		{
			free(pbuffer);
			pbuffer=NULL;
		}
		fclose(fp);
		if(pstr!=NULL)
			return true;
		else
			return false;
		return false;
	}
};
/********************************
	author:chenxuan
	date:2021/9/30
	funtion:find the dir path
*********************************/
bool findDirPath(char* path,char* getPath,char* workName)
{
	char* pfind=NULL,*ptop=path,*pend=strlen(path)+path;
	pfind=strrchr(path,'/');
	if(pfind==NULL)
		return false;
	for(int i=0;ptop!=pfind;i++,ptop++)
		getPath[i]=*ptop;
	for(int i=1;(pfind+i)<=pend;i++)
		workName[i-1]=*(pfind+i);
	return true;	
}
/********************************
	author:chenxuan
	date:2021/9/30
	funtion:find the all name guard
*********************************/
int findWorker(int maxLine,char** buffer)
{
	int num=0;
	FileGet file;
	int len=file.getFileLen("./worker");
	if(len<=0)
	{
		FILE* ftemp=fopen("./worker","w+");
		if(ftemp==NULL)
			return 0;
		fclose(ftemp);
	}
	char* pbuffer=(char*)malloc(sizeof(char)*len+5);
	if(pbuffer==NULL)
		return 0;
	FILE* fp=fopen("./worker","r+");
	if(fp==NULL)
		return 0;
	while(!feof(fp)&&num<maxLine)
	{
		fscanf(fp,"%s",buffer[num]);
		num++;
	}
	fclose(fp);
	free(pbuffer);
	return num;
}
int worker(void*)
{
	char get[128]={0},worker[64]={0},sys[64]={0};
	char** buffer=(char**)malloc(sizeof(char*)*10);
	FileGet file;
	GuardProcess::changeWorkingDir("/root");
	if(buffer==NULL)
		return 0;
	for(int i=0;i<10;i++)
	{
		buffer[i]=(char*)malloc(sizeof(char)*50);
		if(buffer[i]==NULL)
			return 0;
	}
	int num=findWorker(10,buffer)-1;
    printf("num:%d\n",num);
	for(int i=0;i<num;i++)
	{
		findDirPath(buffer[i],get,worker);
		printf("%s %s %s \n",buffer[i],get,worker);
		sprintf(sys,"pstree -p | grep %s > temp",worker);
        printf("sys:%s\n",sys);
		system(sys);
		memset(sys,0,sizeof(char)*64);
		if(false==file.fileStrstr("./temp",worker))
	    {
	    	GuardProcess::changeWorkingDir(get);
	        sprintf(sys,"./%s",worker);
            printf("sys:%s\n",sys);
	        system(sys);
	    }
		memset(get,0,sizeof(char)*128);
		memset(worker,0,sizeof(char)*64);
	}
	for(int i=0;i<10;i++)
		free(buffer[i]);
	free(buffer);
	sleep(10);
	return 0;
}
int main(int argc,char** argv)
{
	GuardProcess proess;
	proess.working(worker,NULL);
	return 0;
}
