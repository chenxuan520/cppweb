#ifndef _SQL_H_
#define _SQL_H_
#include</usr/include/mysql/mysql.h>
class MySql{
private:
    MYSQL* mysql;
    MYSQL one;
    MYSQL_RES* results;
    MySql(MySql&);
    MySql& operator==(MySql&);
public:
    MySql(const char* ipOrHostName,const char* user,const char* passwd,const char* dataBaseName);
    ~MySql();
    int MySqlSelectQuery(const char* sql);
    bool MySqlOtherQuery(const char* sql);
    char** MySqlGetResultRow();
};
#endif

