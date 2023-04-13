#include "../ConnectServer/ConnectServer.h"
#include "../ClientServer/ClientServer.h"
#include "log.h"
#include <iostream>
#include <muduo/base/CurrentThread.h>


int main(int argc, char *argv[])
{
    setLogging(argv[0]);
    if (argc > 1)
    {
        muduo::net::EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        muduo::net::InetAddress serverAddr(port);
        ConnectServer server(&loop, serverAddr);
        if (argc > 2)
        {
            server.setThreadNum(atoi(argv[2]));
            LOG_INFO << "AppServer - "
                      << " -> "
                      << "setThreadNum = " << atoi(argv[2]);
        }
        server.start();
        loop.loop();
        LOG_INFO << "AppServer - "
                  << " -> "
                  << "server start - "<<" -> "<<"loop start";
    }
    else
    {
        printf("Usage: %s port [thread_num]\n", argv[0]);
    }
    return 0;
}