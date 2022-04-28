#ifndef _DEALASK_H_
#define _DEALASK_H_
#include "./cppweb.h"
using namespace cppweb;
//#include"./sql.h"
class DealAsk{
public:
    bool dealGetAsk(ServerTcpIp&,DealHttp&,void*,void*,int);
    bool dealPostAsk(ServerTcpIp&,DealHttp&,void*,void*,int);
    void dealClientIn(ServerTcpIp&,DealHttp&,void*,void*,int);
    void dealClientOut(ServerTcpIp&,DealHttp&,void*,void*,int);
};
#endif
