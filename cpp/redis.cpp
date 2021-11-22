#include <stdio.h>
#include <string.h>
#include <hiredis/hiredis.h>
/********************************
	author:chenxuan
	date:2021/8/15
	funtion:class for deal Redis ask
*********************************/
class RedisDeal{
private:
    redisContext* redis;
    redisReply* reply;
    char command[256];
public:
    RedisDeal(const char* ip,unsigned port )
    {
    	reply=NULL;
    	redis=NULL;
        redis=redisConnect(ip,port);
        memset(command,0,256);
        if(redis==NULL||redis->err)
        	throw redis->errstr;
    }
    ~RedisDeal()
    {
		redisFree(redis);
		if(reply!=NULL)
			freeReplyObject(reply);
	}
	redisReply* doCommand(const char* command)
	{
		if(reply!=NULL)
			freeReplyObject(reply);
		reply=(redisReply*)redisCommand(redis,command);
		return reply;
	}
	const char* get(const char* name)
	{
		if(strlen(name)>200)
			return NULL;
		memset(command,0,256);
		sprintf(command,"get %s",name);
		if(reply!=NULL)
			freeReplyObject(reply);
		reply=(redisReply*)redisCommand(redis,command);
		if(reply->type==REDIS_REPLY_ERROR)
			return NULL;
		return reply->str;
	}
	const char* set(const char* name,const char* value)
	{
		if(strlen(name)+strlen(value)>230)
			return NULL;
		memset(command,0,256);
		sprintf(command,"set %s %s",name,value);
		if(reply!=NULL)
			freeReplyObject(reply);
		reply=(redisReply*)redisCommand(redis,command);
		if(reply->type==REDIS_REPLY_ERROR)
			return NULL;
		return reply->str;
	}
	const char* del(const char* name)
	{
		if(strlen(name)>230)
			return NULL;
		memset(command,0,256);
		sprintf(command,"del %s",name);
		if(reply!=NULL)
			freeReplyObject(reply);
		reply=(redisReply*)redisCommand(redis,command);
		if(reply->type==REDIS_REPLY_ERROR)
			return NULL;
		return reply->str;
	}
	int strLen(const char* name)
	{
		if(strlen(name)>230)
			return -1;
		memset(command,0,256);
		sprintf(command,"strlen %s",name);
		if(reply!=NULL)
			freeReplyObject(reply);
		reply=(redisReply*)redisCommand(redis,command);
		if(reply->type!=REDIS_REPLY_INTEGER)
			return -1;
		return reply->integer;
	}
};
int main()
{
	RedisDeal redis("127.0.0.1",6379);
	redis.set("num","100");
	
    return 0;
}
