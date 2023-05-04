#ifndef _CODE_H_
#define _CODE_H_

#include "../ClientServer/ClientServer.h"
#include <iostream>
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
#include <nlohmann/json.hpp>


enum Session_Type
{
  Request = 1,
  Response = 2,
}

enum Session_Mode
{
  // connect,register,login
  Mode_Connect = 1,
  Mode_Register = 2,
  Mode_Login = 3,
  // chat
  Mode_SendP2P = 4,
  Mode_SendBroad = 5,
  // file、picture
  Mode_SendFile = 6,
  Mode_SendPic = 7,
  // formation set
  Mode_Search = 8,
  Mode_AddFriend = 9,
  Mode_DeleteFriend = 10,
  Mode_ModifyInfo = 11,
};

enum Session_Result
{
  EN_ModeErr = 1,
  EN_Repeated = 2,
  EN_AccountErr = 3,
  EN_PasswdErr = 4,
  EN_Succ = 5,
  EN_Done = 6,
};

enum DBResult{
    DB_Succ = 1,
    DB_Done = 2, // no this user in db
    DB_Error = 3,
};

class LengthHeaderCodec : muduo::noncopyable
{
  typedef std::function<void(const muduo::net::TcpConnectionPtr &, int mode, ClientInfo &)>
      StringMessageCallback;

public:
  explicit LengthHeaderCodec(const StringMessageCallback &cb)
      : messageCallback_(cb) {}

  void onMessage(const muduo::net::TcpConnectionPtr &conn,
                 muduo::net::Buffer *buf,
                 muduo::Timestamp receiveTime)
  {
    muduo::string data = "";
    while (buf->readableBytes() > 0)
      data.append(buf->retrieveAllAsString());
    buf->retrieveAll();
    int32_t length = std::atoi(data.substr(0, kHeaderLen).c_str());
    muduo::string msg = data.substr(kHeaderLen);
    if (length != msg.size())
    {
      LOG_ERROR << "LengthHeaderCodec::onMessage"
                << " -> "
                << "the length of package is error";
    }
    nlohmann::json json_ = nlohmann::json::parse(msg.c_str());
    int mode = json_.at("mode");

    ClientInfo user;
    switch (mode)
    {
    case Mode_Register:
    {
      user.GetAccount() = json_.at("account");
      user.GetPasswd() = json_.at("passwd");
      user.GetNickname() = to_string(json_.at("account"));
      user.GetSex() = "性别未知";
      break;
    }
    case Mode_Login:
    {
      user.GetAccount() = json_.at("account");
      user.GetPasswd() = json_.at("passwd");
      break;
    }
    case Mode_SendP2P:
    {
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      user.GetMessage() = json_.at("message");
      user.GetMsgID() = json_.at("msgID");
      break;
    }
    case Mode_SendBroad:
    {
      //...暂时没有群聊
      break;
    }
    case Mode_SendFile:
    {
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      user.GetMessage() = json_.at("message");
      user.GetMsgID() = json_.at("msgID");
      break;
    }
    case Mode_SendPic:
    {
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      user.GetMessage() = json_.at("message");
      user.GetMsgID() = json_.at("msgID");
      break;
    }
    case Mode_Search:
    {
      user.GetDestination() = json_.at("destination");
      break;
    }
    case Mode_AddFriend:
    {
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      break;
    }
    case Mode_DeleteFriend:
    {
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      break;
    }
    case Mode_ModifyInfo:
    {
      user.GetAccount() = json_.at("account");
      user.GetPasswd() = json_.at("passwd");
      break;
    }
    default:
      break;
    }
    messageCallback_(conn, mode, user);
  }

  static void send(muduo::net::TcpConnection *conn, const muduo::StringPiece &message)
  {
    muduo::net::Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = static_cast<int32_t>(message.size());
    int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
    buf.prepend(&be32, sizeof be32);
    conn->send(&buf);
  }

private:
  StringMessageCallback messageCallback_;
  const static size_t kHeaderLen = sizeof(int32_t); // 4字节包头
};

#endif
