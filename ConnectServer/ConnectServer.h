#ifndef _CONNSER_H_
#define _CONNSER_H_

#include "../Common/code.h"
#include "../Common/DBServer.h"
#include "../ClientServer/ClientServer.h"
#include <unordered_map>
#include <vector>
#include <set>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

enum DBResult{
    DB_Succ = 1,
    DB_Done = 2, // no this user in db
    DB_Error = 3,
};

class ConnectServer : muduo::noncopyable
{
    /* (TcpConnectPtr, account) */
    typedef std::unordered_map<muduo::net::TcpConnectionPtr, muduo::string> ConnectionList;
    typedef std::pair<muduo::net::TcpConnectionPtr, muduo::string> ConnectionPair;

public:
    ConnectServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenADDR);
    void setThreadNum(int numThreads);
    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr &conn);
    void onStringMessage(const muduo::net::TcpConnectionPtr &conn, const int &mode,
                         ClientInfo &userInfo);

    void Login(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user);
    void Register(const muduo::net::TcpConnectionPtr& conn, ClientInfo& user);
    void SendP2P(const muduo::net::TcpConnectionPtr &conn, const muduo::string &source, const muduo::string &destination, const muduo::string &message);

    /* get userdata from mysql */
    DBResult GetUserData(const muduo::string& account, ClientInfo &user);
    /* get friends accountList */
    DBResult GetFriendList(const muduo::string& account, std::vector<ClientInfo>& friendList);
    /* update login time */
    void UpdateLoginTime(const muduo::string& account);

private:
    muduo::net::TcpServer server_;
    LengthHeaderCodec codec_;
    muduo::MutexLock mutex_;
    ConnectionList connections_ GUARDED_BY(mutex_);
};

#endif