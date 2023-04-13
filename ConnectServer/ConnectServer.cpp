#include "ConnectServer.h"

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;
using namespace nlohmann;

ConnectServer::ConnectServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenADDR)
    : server_(loop, listenADDR, "ConnectServer"),
      codec_(std::bind(&ConnectServer::onStringMessage, this, _1, _2, _3))
{
    server_.setConnectionCallback(std::bind(&ConnectServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    std::cout << listenADDR.toIp() << " " << listenADDR.port() << std::endl;
    LOG_INFO << "ConnectServer - "
             << " -> "
             << "listen IP:port = " << listenADDR.toIp() << ":" << listenADDR.port();
}

void ConnectServer::setThreadNum(int numThreads)
{
    server_.setThreadNum(numThreads);
}

void ConnectServer::start()
{
    server_.start();
}

void ConnectServer::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    LOG_INFO << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort()
             << " is " << (conn->connected() ? "UP" : "DOWN");
    // add in connection
    MutexLockGuard lock(mutex_);
    if (conn->connected())
    {
        this->connections_.insert(ConnectionPair(conn, "")); // initial account = 0
        json json_;
        json_["mode"] = Mode_ConnResponse;
        json_["result"] = EN_Succ;
        muduo::string response = json_.dump();
        codec_.send(get_pointer(conn), response);
    }
    else
        this->connections_.erase(conn);
}

void ConnectServer::onStringMessage(const muduo::net::TcpConnectionPtr &conn, const int &mode,
                                    ClientInfo &userInfo)
{
    MutexLockGuard lock(mutex_);
    switch (mode)
    {
    case Mode_Register:
    {
        Register(conn, userInfo);
        break;
    }
    case Mode_Login:
    {
        if (connections_[conn] == "")
        {
            // Repeated Login by "difference conn"
            muduo::string account = userInfo.GetAccount();
            auto iter = std::find_if(connections_.begin(), connections_.end(),
                                     [&account](const std::pair<muduo::net::TcpConnectionPtr, muduo::string> &item)
                                     {
                                         return (item.second == account);
                                     });
            if (iter != connections_.end())
            {
                json json_;
                json_["mode"] = Mode_LoginResponse;
                json_["result"] = EN_Repeated;
                muduo::string response = json_.dump();
                codec_.send(get_pointer(conn), response);
                connections_.erase(conn);
                conn->forceClose();
                return;
            }
            // bind (conn,account)
            connections_[conn] = account;
            this->Login(conn, userInfo);
        }
        else // Repeated Login by "same conn"
        {
            json json_;
            json_["mode"] = Mode_LoginResponse;
            json_["result"] = EN_Repeated;
            muduo::string response = json_.dump();
            codec_.send(get_pointer(conn), response);
            return;
        }
        break;
    }
    case Mode_SendP2P:
    {
        this->SendP2P(conn, userInfo);
        break;
    }
    case Mode_SendBroad:
    {
        //...
        break;
    }
    case Mode_SendFile:
    {
        this->SendFile(conn, userInfo);
        break;
    }
    case Mode_SendPic:
    {
        this->SendPic(conn, userInfo);
        break;
    }
    case Mode_AddFriend:
    {

        break;
    }
    case Mode_DeleteFriend:
    {

        break;
    }
    case Mode_ModifyInfo:
    {

        break;
    }
    default:
    {
        json json_;
        json_["mode"] = Mode_ErrorResponse;
        json_["result"] = EN_ModeErr;
        muduo::string response = json_.dump();
        codec_.send(get_pointer(conn), response);
        break;
    }
    }
}

void ConnectServer::Register(const muduo::net::TcpConnectionPtr &conn, ClientInfo &user)
{
    // find user from db
    MYSQL *mysql = DBServer::MYSQL_INIT(nullptr);
    mysql = DBServer::initDBServer(mysql);
    if (!mysql)
    {
        LOG_ERROR << "ConnectServer::onStringMessage"
                  << " -> "
                  << "Case:Mode_Register"
                  << "->"
                  << "DBServer::initDBServer fail : " << mysql_error(mysql);
        conn->forceClose();
        connections_.erase(conn);
        DBServer::closeDBServer(mysql);
        return;
    }
    char sql[128];
    memset(sql, '0', sizeof(sql));
    sprintf(sql, "select * from userdata where account=%s", user.GetAccount().c_str());
    int ret = DBServer::query(mysql, sql);
    if (ret != 0)
    {
        LOG_ERROR << "ConnectServer::onStringMessage"
                  << " -> "
                  << "Case:Mode_Register"
                  << "->"
                  << "DBServer::query fail : " << mysql_error(mysql);
        conn->forceClose();
        connections_.erase(conn);
        DBServer::closeDBServer(mysql);
        return;
    }
    MYSQL_RES *result = DBServer::store_result(mysql);
    if (result == NULL)
    {
        LOG_ERROR << "ConnectServer::onStringMessage"
                  << " -> "
                  << "Case:Mode_Register"
                  << "->"
                  << "DBServer::store_result fail : " << mysql_error(mysql);
        DBServer::free_result(result);
        DBServer::closeDBServer(mysql);
        return;
    }
    uint64_t rows = DBServer::result_numRow(result);
    if (rows > 0) // find this user -> Reapeted register
    {
        json json_;
        json_["mode"] = Mode_RegisterResponse;
        json_["result"] = EN_Repeated;
        muduo::string response = json_.dump();
        codec_.send(get_pointer(conn), response);
        conn->forceClose();
        connections_.erase(conn);
        DBServer::free_result(result);
        DBServer::closeDBServer(mysql);
        return;
    }
    // register user to db
    memset(sql, '0', sizeof(sql));
    muduo::Timestamp time = muduo::Timestamp::now();
    uint64_t timer = std::atol(time.toString().c_str());

    sprintf(sql, "insert into userdata values('%s','%s','%s','%s','%s','%s',%ld,%ld)", user.GetAccount().c_str(),
            user.GetPasswd().c_str(), user.GetNickname().c_str(), user.GetSex().c_str(), user.GetPhone().c_str(), user.GetEmail().c_str(),timer, timer);
    std::cout << "register sql = " << sql << std::endl;
    ret = DBServer::query(mysql, sql);
    if (ret != 0)
    {
        LOG_ERROR << "ConnectServer::onStringMessage"
                  << " -> "
                  << "Case:Mode_Register"
                  << "->"
                  << "DBServer::query : " << mysql_error(mysql);
        conn->forceClose();
        connections_.erase(conn);
        DBServer::closeDBServer(mysql);
        return;
    }
    json json_;
    json_["mode"] = Mode_RegisterResponse;
    json_["result"] = EN_Succ;
    muduo::string response = json_.dump();
    codec_.send(get_pointer(conn), response);
    DBServer::free_result(result);
    DBServer::closeDBServer(mysql);
}

DBResult ConnectServer::GetUserData(const muduo::string& account, ClientInfo &user)
{
    // get one userdata from mysql
    MYSQL *mysql = DBServer::MYSQL_INIT(nullptr);
    mysql = DBServer::initDBServer(mysql);
    if (!mysql)
    {
        LOG_ERROR << "ConnectServer::GetUserData"
                  << "->"
                  << "DBServer::initDBServer fail : " << mysql_error(mysql);
        DBServer::closeDBServer(mysql);
        return DB_Error;
    }
    char sql[128];
    memset(sql, '0', sizeof(sql));
    sprintf(sql, "select * from userdata where account=%s", account.c_str());
    std::cout<< "Login sql = " <<sql<<std::endl;
    int ret = DBServer::query(mysql, sql);
    if (ret != 0)
    {
        LOG_ERROR << "ConnectServer::GetUserData"
                  << "->"
                  << "DBServer::query fail : " << mysql_error(mysql);
        DBServer::closeDBServer(mysql);
        return DB_Error;
    }
    MYSQL_RES *result = DBServer::store_result(mysql);
    if (result == NULL)
    {
        LOG_ERROR << "ConnectServer::GetUserData"
                  << "->"
                  << "DBServer::store_result fail : " << mysql_error(mysql);
        DBServer::free_result(result);
        DBServer::closeDBServer(mysql);
        return DB_Error;
    }
    // success
    uint64_t rows = DBServer::result_numRow(result);
    if (rows <= 0)
    {
        DBServer::free_result(result);
        DBServer::closeDBServer(mysql);
        return DB_Done;
    }
    MYSQL_ROW rdata = nullptr;
    rdata = DBServer::fetch_row(result);
    if (!rdata)
    {
        LOG_ERROR << "ConnectServer::GetUserData"
                  << "->"
                  << "DBServer::fetch_row fail : " << mysql_error(mysql);
        DBServer::free_result(result);
        DBServer::closeDBServer(mysql);
        return DB_Error;
    }
    user.GetAccount() = rdata[0];
    user.GetPasswd() = rdata[1];
    user.GetNickname() = rdata[2] == nullptr ? "": rdata[2];
    user.GetSex() = rdata[3];
    user.GetPhone() = rdata[4] == nullptr ? "": rdata[4];
    user.GetEmail() = rdata[5] == nullptr ? "": rdata[5];
    user.GetLogintime() = rdata[6];
    user.GetRegistertime() = rdata[7];
    DBServer::free_result(result);
    DBServer::closeDBServer(mysql);
    return DB_Succ;
}

DBResult ConnectServer::GetFriendList(const muduo::string& account, std::vector<ClientInfo>& friendList)
{
    MYSQL *mysql = DBServer::MYSQL_INIT(nullptr);
    mysql = DBServer::initDBServer(mysql);
    if (!mysql)
    {
        LOG_ERROR << "ConnectServer::GetFriendList"
                  << "->"
                  << "DBServer::initDBServer fail : " << mysql_error(mysql);
        DBServer::closeDBServer(mysql);
        return DB_Error;
    }

    /* get friend account */
    char sql[128];
    memset(sql, '0', sizeof(sql));
    sprintf(sql, "select id_1,id_2 from user_relationship where id_1=%s || id_2=%s", account.c_str(), account.c_str());
    int ret = DBServer::query(mysql, sql);
    if (ret != 0)
    {
        LOG_ERROR << "ConnectServer::GetFriendList"
                  << "->"
                  << "DBServer::query fail : " << mysql_error(mysql);
        DBServer::closeDBServer(mysql);
        return DB_Error;
    }
    MYSQL_RES *result = DBServer::store_result(mysql);
    if (result == NULL)
    {
        LOG_ERROR << "ConnectServer::GetFriendList"
                  << "->"
                  << "DBServer::store_result fail : " << mysql_error(mysql);
        DBServer::free_result(result);
        DBServer::closeDBServer(mysql);
        return DB_Error;
    }
    uint64_t rows = DBServer::result_numRow(result);
    uint64_t nums = DBServer::field_count(mysql);
    if (rows <= 0)
    {
        DBServer::free_result(result);
        DBServer::closeDBServer(mysql);
        return DB_Done;
    }
    std::set<muduo::string> set_;
    for(int i = 0; i< rows; i++)
    {
        MYSQL_ROW rdata = DBServer::fetch_row(result);
        for(int j=0; j < nums; j++)
        {
            if(account != muduo::string(rdata[j]))
                set_.insert(muduo::string(rdata[j]));
        }
    }
    DBServer::free_result(result);
    DBServer::closeDBServer(mysql);
    if(set_.empty())
        return DB_Done;
    /* get friends info */
    for(auto iter : set_)
    {
        ClientInfo user;
        if(GetUserData(iter, user) == DB_Succ)
            friendList.push_back(user);
    }
    if(friendList.empty())
        return DB_Done;
    return DB_Succ;
}

void ConnectServer::Login(const muduo::net::TcpConnectionPtr &conn, ClientInfo &user)
{
    /* get user data */
    ClientInfo DBUser;
    int udate_ret = GetUserData(user.GetAccount(), DBUser);
    if(udate_ret == DB_Done)
    {
        json json_;
        json_["mode"] = Mode_LoginResponse;
        json_["result"] = EN_AccountErr;
        muduo::string response = json_.dump();
        codec_.send(get_pointer(conn), response);
        conn->forceClose();
        connections_.erase(conn);
        return;
    }
    else if(udate_ret == DB_Error)
    {
        conn->forceClose();
        connections_.erase(conn);
        return;
    }
    /* check passwd */
    if (user.GetPasswd() != DBUser.GetPasswd())
    {
        json json_;
        json_["mode"] = Mode_LoginResponse;
        json_["result"] = EN_PasswdErr;
        muduo::string response = json_.dump();
        codec_.send(get_pointer(conn), response);
        conn->forceClose();
        connections_.erase(conn);
        return;
    }
    json json_;
    json_["mode"] = Mode_LoginResponse;
    json_["result"] = EN_Succ;
    json_["account"] = DBUser.GetAccount();
    json_["nickname"] = DBUser.GetNickname();
    json_["sex"] = DBUser.GetSex();
    json_["phone"] = DBUser.GetPhone();
    json_["email"] = DBUser.GetEmail();
    /* get friends accountList */
    std::vector<ClientInfo> friendList;
    int friend_ret = GetFriendList(user.GetAccount(),friendList);
    if(friend_ret == DB_Done || friend_ret == DB_Error)
        json_["friendnum"] = 0;
    else
    {
        size_t friendNum = friendList.size();
        json_["friendnum"] = friendNum;
        for(size_t i=0; i<friendList.size(); i++)
            json_["friendList"][i] = {{"account", friendList[i].GetAccount()},
                                     {"nickname", friendList[i].GetNickname()},
                                     {"phone", friendList[i].GetPhone()},
                                     {"sex", friendList[i].GetSex()},
                                     {"email", friendList[i].GetEmail()}};
    }
    // send response
    muduo::string response = json_.dump();
    codec_.send(get_pointer(conn), response);
}

void ConnectServer::UpdateLoginTime(const muduo::string& account)
{
    // update login time
    MYSQL *mysql = DBServer::MYSQL_INIT(nullptr);
    mysql = DBServer::initDBServer(mysql);
    if (!mysql)
    {
        LOG_ERROR << "ConnectServer::GetFriendList"
                  << "->"
                  << "DBServer::initDBServer fail : " << mysql_error(mysql);
        DBServer::closeDBServer(mysql);
        return;
    }
    muduo::Timestamp time = muduo::Timestamp::now();
    uint64_t timer = std::atol(time.toString().c_str());
    char sql[128];
    memset(sql, '0', sizeof(sql));
    sprintf(sql,"UPDATE userdata SET login_time=%ld where account = %s", timer, account.c_str());
    int ret = DBServer::query(mysql, sql);
    if (ret != 0)
        LOG_ERROR << "ConnectServer::onStringMessage"
                  << " -> "
                  << "update loginTime"
                  << "->"
                  << "DBServer::query fail : " << mysql_error(mysql);
    DBServer::closeDBServer(mysql);
}

void ConnectServer::SendP2P(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user)
{
    if (connections_[conn] != "")
    {
        /* find connPtr(key) by destination(value) */
        std::unordered_map<muduo::net::TcpConnectionPtr, muduo::string>::iterator iter = connections_.end();
        iter = std::find_if(connections_.begin(), connections_.end(),
                            [&user](const std::pair<muduo::net::TcpConnectionPtr, muduo::string> &item)
                            {
                                return (item.second == user.GetDestination());
                            });
        if (iter != connections_.end()) // destination online
        {
            // send data to destination
            json json_;
            json_["mode"] = Mode_SendP2P;
            json_["result"] = EN_Succ;
            json_["source"] = user.GetSource();
            json_["destination"] = user.GetDestination();
            json_["message"] = user.GetMessage();
            json_["msgID"] = user.GetMsgID();
            muduo::string data = json_.dump();
            codec_.send(get_pointer(iter->first), data);

            // send response to source
            json_.clear();
            json_["mode"] = Mode_ErrorResponse;
            json_["result"] = EN_Succ;
            json_["msgID"] = user.GetMsgID();
            muduo::string response = json_.dump();
            codec_.send(get_pointer(conn), response);
        }
        else // destination not online
        {
            json json_;
            json_["mode"] = Mode_ErrorResponse;
            json_["result"] = EN_Done;
            json_["msgID"] = user.GetMsgID();
            muduo::string response = json_.dump();
            codec_.send(get_pointer(conn), response);
        }
    }
}

void ConnectServer::SendFile(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user)
{
    if (connections_[conn] != "")
    {
        /* find connPtr(key) by destination(value) */
        std::unordered_map<muduo::net::TcpConnectionPtr, muduo::string>::iterator iter = connections_.end();
        iter = std::find_if(connections_.begin(), connections_.end(),
                            [&user](const std::pair<muduo::net::TcpConnectionPtr, muduo::string> &item)
                            {
                                return (item.second == user.GetDestination());
                            });
        if (iter != connections_.end()) // destination online
        {
            // send data to destination
            json json_;
            json_["mode"] = Mode_SendFile;
            json_["result"] = EN_Succ;
            json_["source"] = user.GetSource();
            json_["destination"] = user.GetDestination();
            json_["message"] = user.GetMessage();
            json_["msgID"] = user.GetMsgID();
            muduo::string data = json_.dump();  //序列化
            codec_.send(get_pointer(iter->first), data);

            // send response to source
            json_.clear();
            json_["mode"] = Mode_FileResponse;
            json_["result"] = EN_Succ;
            json_["msgID"] = user.GetMsgID();
            muduo::string response = json_.dump();
            codec_.send(get_pointer(conn), response);
        }
        else // destination not online
        {
            json json_;
            json_["mode"] = Mode_FileResponse;
            json_["result"] = EN_Done;
            json_["msgID"] = user.GetMsgID();
            muduo::string response = json_.dump();
            codec_.send(get_pointer(conn), response);
        }
    }
}

void ConnectServer::SendPic(const muduo::net::TcpConnectionPtr &conn, ClientInfo& user)
{
    if (connections_[conn] != "")
    {
        /* find connPtr(key) by destination(value) */
        std::unordered_map<muduo::net::TcpConnectionPtr, muduo::string>::iterator iter = connections_.end();
        iter = std::find_if(connections_.begin(), connections_.end(),
                            [&user](const std::pair<muduo::net::TcpConnectionPtr, muduo::string> &item)
                            {
                                return (item.second == user.GetDestination());
                            });
        if (iter != connections_.end()) // destination online
        {
            // send data to destination
            json json_;
            json_["mode"] = Mode_SendPic;
            json_["result"] = EN_Succ;
            json_["source"] = user.GetSource();
            json_["destination"] = user.GetDestination();
            json_["message"] = user.GetMessage();
            json_["msgID"] = user.GetMsgID();
            muduo::string data = json_.dump();  //序列化
            codec_.send(get_pointer(iter->first), data);

            // send response to source
            json_.clear();
            json_["mode"] = Mode_PicResponse;
            json_["result"] = EN_Succ;
            json_["msgID"] = user.GetMsgID();
            muduo::string response = json_.dump();
            codec_.send(get_pointer(conn), response);
        }
        else // destination not online
        {
            json json_;
            json_["mode"] = Mode_PicResponse;
            json_["result"] = EN_Done;
            json_["msgID"] = user.GetMsgID();
            muduo::string response = json_.dump();
            codec_.send(get_pointer(conn), response);
        }
    }
}