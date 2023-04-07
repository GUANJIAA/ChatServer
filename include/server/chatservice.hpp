#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "json.hpp"
#include "redis.hpp"
#include "usermodel.hpp"
#include "groupmodel.hpp"
#include "friendmodel.hpp"
#include "offlinemessagemodel.hpp"

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>

using namespace std;
using namespace muduo::net;
using namespace muduo;

using json = nlohmann::json;

// 表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &, json &, Timestamp)>;

// 聊天服务器业务类
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();

    // 处理登录业务
    void login(const TcpConnectionPtr &, json &, Timestamp);
    // 注销用户业务
    void loginout(const TcpConnectionPtr &, json &, Timestamp);
    // 处理注册业务
    void reg(const TcpConnectionPtr &, json &, Timestamp);
    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr &, json &, Timestamp);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &, json &, Timestamp);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &, json &, Timestamp);
    // 加入群组业务
    void addGroup(const TcpConnectionPtr &, json &, Timestamp);
    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &, json &, Timestamp);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    // 服务器异常退出，业务重置方法
    void reset();
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);

private:
    ChatService();

    // 存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    OfflineMsgModel _offlineMsgModel;

    // redis操作对象
    Redis _redis;
};

#endif