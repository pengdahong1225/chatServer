#ifndef SERVER_H
#define SERVER_H

#include "code.h"
#include <unordered_map>
#include <iostream>
#include <unistd.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

class chatServer : muduo::noncopyable
{
public:
    chatServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenADDR);
    void setThreadNum(int numThreads);
    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr &conn);
    void onStringMessage(const muduo::net::TcpConnectionPtr &conn, const int &mode,
                         const int &id_1, const int &id_2, const muduo::string &message, muduo::Timestamp);
    /* typedef std::shared_ptr<TcpConnection> TcpConnectionPtr */
    typedef std::unordered_map<muduo::net::TcpConnectionPtr, int> ConnectionList;
    typedef std::pair<muduo::net::TcpConnectionPtr, int> ConnectionPair;
    muduo::net::TcpServer server_;
    LengthHeaderCodec codec_;
    muduo::MutexLock mutex_;
    ConnectionList connections_ GUARDED_BY(mutex_);
};

#endif // SERVER_H