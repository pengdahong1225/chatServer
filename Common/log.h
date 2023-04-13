#pragma once

#include <muduo/base/AsyncLogging.h>
#include <iostream>
#include <muduo/base/Logging.h>

int kRollSize = 500 * 1000 * 1000;
std::unique_ptr<muduo::AsyncLogging> g_asyncLog;
void asyncOutput(const char *msg, int len)
{
    g_asyncLog->append(msg, len);
}
void setLogging(const char *argv0)
{
    muduo::Logger::setOutput(asyncOutput);
    char name[256];
    strncpy(name, argv0, 256);
    g_asyncLog.reset(new muduo::AsyncLogging(::basename(name), kRollSize));
    g_asyncLog->start();
}