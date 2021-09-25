#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
/********************************
	author:chenxuan
	date:2021/9/8
	funtion:the class for create guard process
	parameter:isAllHide stand for use hide all mode
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
int func(void* guardName)
{
	char sys[50]={0};
	const char* name=(char*)guardName;
	FileGet file;
	if(strlen(name)==0)
		exit(0);
	sleep(10);
    printf("guard:%s\n",name);
	sprintf(sys,"pstree -p | grep %s > temp",name);
	system(sys);
    memset(sys,0,sizeof(char)*50);
	if(false==file.fileStrstr("./temp",name))
    {
        sprintf(sys,"./%s",name);
        system(sys);
    }
	return 0;
}
int main(int argc,char** argv)
{
	char nameGuard[20]={0};
	if(argc==1)
	{
		printf("please input the name of guard:");
		scanf("%s",nameGuard);
	}
	GuardProcess process;
	if(argc>1)
    {
        printf("argv %s\n",argv[1]);
		process.working(func,argv[1]);
    }
	else 
		process.working(func,nameGuard);
	return 0;
}
