#include "../header/server.h"
#include <iostream>
#include <muduo/base/CurrentThread.h>

int main(int argc, char *argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 1)
    {
        muduo::net::EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        muduo::net::InetAddress serverAddr(port);
        chatServer server(&loop, serverAddr);
        if (argc > 2)
        {
            server.setThreadNum(atoi(argv[2]));
        }
        server.start();
        loop.loop();
    }
    else
    {
        printf("Usage: %s port [thread_num]\n", argv[0]);
    }
    return 0;
}