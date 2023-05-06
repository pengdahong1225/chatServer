#include "ClientServer.h"

using namespace muduo;

muduo::string &ClientInfo::GetAccount()
{
    return account;
}

muduo::string &ClientInfo::GetPasswd()
{
    return passwd;
}

muduo::string &ClientInfo::GetNickname()
{
    return nickname;
}

muduo::string &ClientInfo::GetSex()
{
    return sex;
}

muduo::string& ClientInfo::GetPhone()
{
    return phone;
}

muduo::string& ClientInfo::GetEmail()
{
    return email;
}

muduo::string &ClientInfo::GetLogintime()
{
    return loginime;
}

muduo::string &ClientInfo::GetRegistertime()
{
    return registertime;
}

muduo::string &ClientInfo::GetSource()
{
    return source;
}

muduo::string &ClientInfo::GetDestination()
{
    return destination;
}

muduo::string& ClientInfo::GetMessage()
{
    return message;
}
muduo::string& ClientInfo::GetMsgID()
{
    return messageID;
}

int& ClientInfo::GetResult()
{
    return result;
}

muduo::string& ClientInfo::GetFileName()
{
    return filename;
}

int& ClientInfo::GetNumPiece()
{
    return num_piece;
}

int& ClientInfo::GetPiece()
{
    return piece;
}