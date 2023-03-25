#ifndef _DBSERVER_H_
#define _DBSERVER_H_

#include "../Common/singleton.h"
#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
#include <unistd.h>
#include <string>

struct DBConfig{
    const char* ip = "rm-2vcnd09f392wgd9hoho.rwlb.cn-chengdu.rds.aliyuncs.com"; //阿里云 RDS
    const char* user = "messi_pengdahong";
    const char* passwd = "1225Gkl_";
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
    static struct DBConfig addr_;
};

#endif