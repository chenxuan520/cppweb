#include "../hpp/config.h"
int main()
{
	clock_t start = clock();
	for(unsigned i=0;i<100000;i++)
		LogSystem::recordRequest("test log",0);
	clock_t ends = clock();
	std::cout <<"Running Time : "<<100000/((double)(ends - start)/ CLOCKS_PER_SEC)<<std::endl;
	return 0;
}
