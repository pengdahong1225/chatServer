#ifndef _CLIENTSER_H_
#define _CLIENTSER_H_

#include <unordered_map>
#include <iostream>
#include <unistd.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

class ClientInfo
{
public:
    // all return reference
    muduo::string& GetAccount();
    muduo::string& GetPasswd();
    muduo::string& GetNickname();
    muduo::string& GetSex();
    muduo::string& GetPhone();
    muduo::string& GetEmail();
    muduo::string& GetLogintime();
    muduo::string& GetRegistertime();

    muduo::string& GetSource();
    muduo::string& GetDestination();
    muduo::string& GetMessage();
    muduo::string& GetMsgID();
    muduo::string& GetFileName();
    int& GetNumPiece();
    int& GetPiece();

    int& GetResult();
private:
    muduo::string account;
    muduo::string passwd;
    muduo::string nickname;
    muduo::string sex="性别未知";
    muduo::string phone="";
    muduo::string email="";
    muduo::string loginime;
    muduo::string registertime;

    muduo::string source;
    muduo::string destination;
    muduo::string message;
    muduo::string filename;
    int num_piece = 0;
    int piece = 0;

    muduo::string messageID; //客户端要用，服务端不用管直接转发

    int result = -1;
public:
    std::vector<muduo::string> friendList;
};

#endif