#ifndef _DBSERVER_H_
#define _DBSERVER_H_

#include "../Common/singleton.h"
#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
#include <unistd.h>
#include <string>

struct DBaddr{
    const char* ip = "162.14.116.63"; //tencent
    const char* user = "root";
    const char* passwd = "1225";
    const char* database = "chatServer";
};

class DBServer : muduo::noncopyable
{
public:
    static MYSQL* MYSQL_INIT(MYSQL* mysql);
    static MYSQL* initDBServer(MYSQL* mysql);
    static void closeDBServer(MYSQL* mysql);
    static int query(MYSQL* mysql,const char* sql);//success : 0
    static MYSQL_RES* store_result(MYSQL* mysql);
    static MYSQL_ROW fetch_row(MYSQL_RES *result);
    static uint64_t result_numRow(MYSQL_RES* result);
    static int field_count(MYSQL* mysql);
    static void free_result(MYSQL_RES* result);
private:
    static struct DBaddr addr_;
};

#endif