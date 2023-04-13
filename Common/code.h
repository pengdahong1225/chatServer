#ifndef _CODE_H_
#define _CODE_H_

#include "../ClientServer/ClientServer.h"
#include "log.h"
#include <iostream>
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
#include <json.cpp>

enum Session_Mode
{
  //  connect,register,login
  Mode_Register = 1,
  Mode_Login = 2,
  Mode_LoginResponse = 3,
  Mode_RegisterResponse = 4,
  Mode_ConnResponse = 5,
  // chat
  Mode_SendP2P = 6,
  Mode_P2PResponse = 7,
  Mode_SendBroad = 8,
  Mode_BroadResponse = 9,
  // file、picture
  Mode_SendFile = 10,
  Mode_FileResponse = 11,
  Mode_SendPic = 12,
  Mode_PicResponse = 13,
  // formation set
  Mode_AddFriend = 14,
  Mode_DeleteFriend = 15,
  Mode_ModifyInfo = 16,
  Mode_ErrorResponse = 17,
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
    /* decode */
    nlohmann::json json_ = nlohmann::json::parse(msg.c_str());
    int mode = json_.at("mode");
    switch (mode)
    {
    case Mode_Register:
    {
      ClientInfo user;
      user.GetAccount() = json_.at("account");
      user.GetPasswd() = json_.at("passwd");
      user.GetNickname() = to_string(json_.at("account"));
      user.GetSex() = "性别未知";
      messageCallback_(conn, mode, user);
      break;
    }
    case Mode_Login:
    {
      ClientInfo user;
      user.GetAccount() = json_.at("account");
      user.GetPasswd() = json_.at("passwd");
      messageCallback_(conn, mode, user);
      break;
    }
    case Mode_SendP2P:
    {
      ClientInfo user;
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      user.GetMessage() = json_.at("message");
      user.GetMsgID() = json_.at("msgID");
      messageCallback_(conn, mode, user);
      break;
    }
    case Mode_SendBroad:
    {
      //...暂时没有群聊
      break;
    }
    case Mode_SendFile:
    {
      ClientInfo user;
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      user.GetMessage() = json_.at("message");
      user.GetMsgID() = json_.at("msgID");
      messageCallback_(conn, mode, user);
      break;
    }
    case Mode_SendPic:
    {
      ClientInfo user;
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      user.GetMessage() = json_.at("message");
      user.GetMsgID() = json_.at("msgID");
      messageCallback_(conn, mode, user);
      break;
    }
    case Mode_AddFriend:
    {
      ClientInfo user;
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      messageCallback_(conn, mode, user);
      break;
    }
    case Mode_DeleteFriend:
    {
      ClientInfo user;
      user.GetSource() = json_.at("source");
      user.GetDestination() = json_.at("destination");
      messageCallback_(conn, mode, user);
      break;
    }
    case Mode_ModifyInfo:
    {
      ClientInfo user;
      user.GetAccount() = json_.at("account");
      user.GetPasswd() = json_.at("passwd");
      messageCallback_(conn, mode, user);
      break;
    }
    default:
      break;
    }
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
