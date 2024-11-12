#include "HttpServer.h"
#include "HttpParse.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "TcpClient.h"
#include "logger.h"
#include "HttpParse.h"
#include "Buffer.h"
#include <gflags/gflags.h>

// 定义一个用于设置线程数的 gflags 参数，默认值为 4
DEFINE_int32(threadNum, 4, "set thread num");

using namespace std;

// HttpServer 类的成员函数实现

// 读取回调函数，处理客户端发来的数据
void HttpServer::ReadCallback(Pine::clientPtr client, Buffer* inputBuffer) {
    // 获取客户端发来的请求数据
    string str = inputBuffer->getAllString();
    LOG_INFO("%s", str.c_str());
    
    // 获取该客户端的 HttpInfo 对象，并重置状态
    unique_ptr<HttpInfo>& httpInfo = client->getHttpInfo();
    httpInfo->reset();
    
    // 分析请求消息并填充 HttpInfo 对象
    httpInfo->request_->analyseFile(str, httpInfo);
    
    // 设置响应头并发送请求的文件（调用 HttpResponse 进行文件发送）
    httpInfo->response_->SendFile(client.get(), httpInfo);
}

// 启动服务器
void HttpServer::run() {
    // 设置线程数
    tcpServer_.setThreadNum(FLAGS_threadNum);
    LOG_INFO("conf threadNum:%d", FLAGS_threadNum);
    
    // 设置客户端数据读取回调函数
    tcpServer_.setClientReadCallback(std::bind(&HttpServer::ReadCallback, this, std::placeholders::_1, std::placeholders::_2));
    
    // 启动 TCP 服务器
    tcpServer_.start();
    
    // 进入事件循环，开始处理事件
    loop_.loop();
}

// 析构函数，默认析构
HttpServer::~HttpServer() = default;

// 构造函数，初始化事件循环和 TCP 服务器
HttpServer::HttpServer() : loop_(), tcpServer_(&loop_), postCallback_() {}
