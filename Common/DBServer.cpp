#include "DBServer.h"
struct DBConfig DBServer::addr_;

MYSQL* DBServer::MYSQL_INIT(MYSQL* mysql)
{
    return mysql_init(mysql);
}

MYSQL* DBServer::initDBServer(MYSQL* mysql)
{
    return mysql_real_connect(mysql,addr_.ip,addr_.user,addr_.passwd,addr_.database,3306,nullptr,0);
}

void DBServer::closeDBServer(MYSQL* mysql)
{
    mysql_close(mysql);
}

int DBServer::query(MYSQL* mysql,const char* sql)
{
    return mysql_query(mysql,sql);
}

MYSQL_RES* DBServer::store_result(MYSQL* mysql)
{
    return mysql_store_result(mysql);
}

uint64_t DBServer::result_numRow(MYSQL_RES* result)
{
    return mysql_num_rows(result);
}

MYSQL_ROW DBServer::fetch_row(MYSQL_RES *result)
{
    return mysql_fetch_row(result);
}

int DBServer::field_count(MYSQL* mysql)
{
    return mysql_field_count(mysql);
}

void DBServer::free_result(MYSQL_RES* result)
{
    mysql_free_result(result);
}