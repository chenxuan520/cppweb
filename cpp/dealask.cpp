/********************************
	author:chenxuan
	date:2021.7.5
	funtion:this cpp is for you to deal ask 
	if you wang to change you web to a no-static
*********************************/
#include"../hpp/dealask.h"
#include"../hpp/http.h"
#include"../hpp/server.h"
//#include"../hpp/sql.h"
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:this function is to deal get you can
	add hpp and add funtion there
*********************************/
bool DealAsk::dealGetAsk(ServerTcpIp& server,DealHttp& http,void* pget,void* sen,int soc)
{
	return false;
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:deal post ask there
*********************************/
bool DealAsk::dealPostAsk(ServerTcpIp& server,DealHttp& http,void* pget,void* sen,int soc)
{
	return true;
}
void DealAsk::dealClientIn(ServerTcpIp& server,DealHttp& http,void* pget,void* sen,int soc)
{
	return;
}
void DealAsk::dealClientOut(ServerTcpIp& server,DealHttp& http,void* pget,void* sen,int soc)
{
	return;
}
/********************************
	author:chenxuan
	date:2021/11/7
	funtion:funtion for 2.0 server add handle
*********************************/
bool addHandle(HttpServer& server)
{
	
}
