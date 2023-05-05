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

    /* 具体功能 */
    void Login(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user);
    void Register(const muduo::net::TcpConnectionPtr& conn, ClientInfo& user);
    void SendP2P(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user);
    void SendFile(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user);
    void SendPic(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user);
    void Search(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user);
    void Addfriend(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user);
    void CheckAddFriend(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user);//同意/拒绝添加为好友
    void DeleteFriend(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user);

    /* get userdata from mysql */
    DBResult GetUserData(const muduo::string& account, ClientInfo &user);
    /* get friends accountList */
    DBResult GetFriendList(const muduo::string& account, std::vector<ClientInfo>& friendList);
    /* update login time */
    void UpdateLoginTime(const muduo::string& account);

    bool FindUser(ClientInfo &user);
    

private:
    muduo::net::TcpServer server_;
    LengthHeaderCodec codec_;
    muduo::MutexLock mutex_;
    ConnectionList connections_ GUARDED_BY(mutex_);
};

#endif