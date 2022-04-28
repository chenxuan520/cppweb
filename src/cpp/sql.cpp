#include "../hpp/sql.h"
#include</usr/include/mysql/mysql.h>
#include<iostream>
using namespace std;
MySql::MySql(const char* ipOrHostName,const char* user,const char* passwd,const char* dataBaseName)
{
    this->results=NULL;
    this->mysql=mysql_init(NULL);
    if(NULL==mysql_real_connect(this->mysql,ipOrHostName,user,passwd,dataBaseName,0,NULL,0))
    {
        cerr<<mysql_error(this->mysql)<<endl;
        this->~MySql();
        throw;
    }
}
MySql::~MySql()
{
    if(this->results!=NULL)
        mysql_free_result(this->results);
    mysql_close(mysql);
}
int MySql::MySqlSelectQuery(const char* sql)
{
    if(this->results!=NULL)
        mysql_free_result(this->results);
    int temp=mysql_query(this->mysql,sql);
    if(temp!=0)
    {
        cerr<<mysql_error(this->mysql)<<endl;
        return 0;
    }
    this->results=mysql_store_result(mysql);
    if(results==NULL)
    {
        cerr<<mysql_error(this->mysql)<<endl;
        return 0;
    }
    return mysql_num_fields(this->results);
}
bool MySql::MySqlOtherQuery(const char* sql)
{
    int temp=mysql_query(this->mysql,sql);
    if(temp!=0)
    {
        cerr<<mysql_error(this->mysql)<<endl;
        return false;
    }
    return true;
}
char** MySql::MySqlGetResultRow()
{
    if(this->results==NULL)
        return NULL;
    return mysql_fetch_row(results);
}

