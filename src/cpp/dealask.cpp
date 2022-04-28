/********************************
	author:chenxuan
	date:2021.7.5
	funtion:this cpp is for you to deal ask 
	if you wang to change you web to a no-static
*********************************/
#include"../hpp/dealask.h"
#include "../hpp/cppweb.h"
//#include"../hpp/sql.h"
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:this function is to deal get you can
	add hpp and add funtion there
*********************************/
bool DealAsk::dealGetAsk(ServerTcpIp& ,DealHttp& ,void* ,void*,int )
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
