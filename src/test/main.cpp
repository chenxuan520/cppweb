#include "test.h"
#include "../hpp/cppweb.h"
#include "dealhttp_test.h"
#include "client_test.h"
#include "onlinetest.h"

INIT(init){
	DEBUG("Initing")
	return 0;
}

TEST(TestForHeader, RunTest){
	MUST_EQUAL(1, 1);
}

RUN
