#include "test.h"
#include "../hpp/cppweb.h"
#include "dealhttp_test.h"

INIT(init){
	printf("Runing INIT\n\n");
	return 0;
}

TEST(TestForHeader, RunTest){
	MUST_EQUAL(1, 1);
}


RUN
