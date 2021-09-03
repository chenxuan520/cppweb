#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
class GuardProcess{
private:
	int pid;
public:
	GuardProcess()
	{
		pid=0;
		pid=fork();
		if(pid!=0)
			exit(0);
		pid=setsid();
		if(-1==pid)
		{
			perror("pid");
			exit(0);
		}
		pid=chdir("/");
		if(-1==pid)
		{
			perror("chdir");
			exit(0);
		}
		umask(0);
		close(STDERR_FILENO);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
	}
	void working(int (*pfuncWork)(void*),void* arg)
	{
		while(pfuncWork(arg)==0);	
	}
};
int func(void*)
{
	system("date > /data/time");
	return 0;
}
int main()
{
	GuardProcess process;
	process.working(func,NULL);
}
