#pragma once
#include <string>
#include <unordered_map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include "TcpClientCallback.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "HttpCallback.h"

// 前置声明，避免不必要的依赖
class TcpClient;
class Buffer;
class HttpInfo;

class HttpServer {
private:
    // 使用函数对象表示回调函数类型，接收一个 HttpInfo 指针作为参数
    using callback = std::function<void(HttpInfo*)>;

private:
    EventLoop loop_;        // 主事件循环对象，用于事件调度
    TcpServer tcpServer_;   // TCP 服务器对象，处理客户端连接
    callback postCallback_; // 用户自定义的 POST 请求回调函数

public:
    // 默认构造函数，初始化事件循环和 TCP 服务器
    HttpServer();

    // 读取回调函数，处理客户端请求数据并发送响应
    void ReadCallback(Pine::clientPtr t, Buffer* inputBuffer);

    // 启动服务器的运行，启动事件循环和 TCP 服务器
    void run();

    // 设置用户自定义的 POST 请求回调函数
    void setPostReadCallback(const char* s, callback cb) {
        HttpCallback::setPostCB(s, cb);
    }

    // 析构函数
    ~HttpServer();
};
