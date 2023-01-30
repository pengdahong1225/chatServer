#include "../header/server.h"

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;

chatServer::chatServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenADDR)
    : server_(loop, listenADDR, "chatServer"),
      codec_(std::bind(&chatServer::onStringMessage, this, _1, _2, _3, _4, _5, _6))
{
    server_.setConnectionCallback(std::bind(&chatServer::onConnection, this, _1));
    /* 先回调解码LengthHeaderCodec::onMessage，
        再回调ChatServer::onStringMessage */
    server_.setMessageCallback(std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    std::cout << listenADDR.toIp() << " " << listenADDR.port() << std::endl;
}

void chatServer::setThreadNum(int numThreads)
{
    server_.setThreadNum(numThreads);
    printf("numThreads = %d\n", numThreads);
}

void chatServer::start()
{
    server_.start();
}

void chatServer::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    /* socket connected */
    LOG_INFO << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort()
             << " is " << (conn->connected() ? "UP" : "DOWN");
    MutexLockGuard lock(mutex_);
    if (conn->connected())
        this->connections_.insert(ConnectionPair(conn, -1));
    else
        this->connections_.erase(conn);
}

void chatServer::onStringMessage(const muduo::net::TcpConnectionPtr &conn, const int &mode,
                                 const int &id_1, const int &id_2, const muduo::string &message, muduo::Timestamp)
{
    MutexLockGuard lock(mutex_);
    if (mode == 0) // login
    {
        if (connections_[conn] == -1)
            connections_[conn] = id_1;
        else
            codec_.send(get_pointer(conn), muduo::string("mode error!\r\n"));
    }
    else if (mode == 1) // message
    {
        if (connections_[conn] != -1)
        {
            std::unordered_map<muduo::net::TcpConnectionPtr, int>::iterator iter = connections_.end();
            iter = std::find_if(connections_.begin(), connections_.end(),
                                [&](const std::pair<muduo::net::TcpConnectionPtr, int> &item)
                                {
                                    return (item.second == id_2);
                                });
            if (iter != connections_.end())
                codec_.send(get_pointer(iter->first), message);
            else
                codec_.send(get_pointer(conn), muduo::string("not online!\r\n"));
        }
        else
            codec_.send(get_pointer(conn), muduo::string("un login!\r\n"));
    }
    else
        codec_.send(get_pointer(conn), muduo::string("message error!\r\n"));
}