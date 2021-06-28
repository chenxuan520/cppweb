#ifndef _DEALASK_H_
#define _DEALASK_H_
#include"./http.h"
#include"./server.h"
#include"./sql.h"
class DealAsk{
public:
    bool dealGetAsk(ServerTcpIp& server,DealHttp& http);
    bool dealPostAsk(ServerTcpIp& server,DealHttp& http);
};
#endif
