#ifndef MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
#define MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H

#include <iostream>
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
#include <nlohmann/json.hpp>

class LengthHeaderCodec : muduo::noncopyable
{
public:
  typedef std::function<void(const muduo::net::TcpConnectionPtr &, const int &mode,
                             const int &id_1, const int &id_2, const muduo::string &message,
                             muduo::Timestamp)>
      StringMessageCallback;

  explicit LengthHeaderCodec(const StringMessageCallback &cb)
      : messageCallback_(cb) {}

  void onMessage(const muduo::net::TcpConnectionPtr &conn,
                 muduo::net::Buffer *buf,
                 muduo::Timestamp receiveTime)
  {
    muduo::string msg = "";
    while (buf->readableBytes() > 0)
      msg.append(buf->retrieveAllAsString());
    buf->retrieveAll();
    // std::cout << msg << std::endl;
    /* decode */
    nlohmann::json json_ = nlohmann::json::parse(msg.c_str());
    int mode = json_.at("mode");
    int id_1 = json_.at("id_1");
    int id_2 = json_.at("id_2");
    muduo::string message = json_.at("data");
    messageCallback_(conn, mode, id_1, id_2, message, receiveTime);
  }

  void send(muduo::net::TcpConnection *conn, const muduo::StringPiece &message)
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
};

#endif // MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
