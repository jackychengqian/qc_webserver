#pragma once
#include <string>
#include <memory>
#include <functional>
#include "const.h"
#include "document.h"

// 前向声明，避免循环依赖
class TcpClient;
struct ResponseHead;
struct RequestFileInfo;
struct HttpParse;
class HttpInfo;
class Header;

class HttpResponse
{
public:
    // 构造函数：初始化响应对象
    HttpResponse();
    // 析构函数：释放资源
    ~HttpResponse();

    // 发送文件响应到客户端
    void SendFile(TcpClient* t, std::unique_ptr<HttpInfo>&);

    // 重置响应对象的状态
    void reset();

    // 设置 HTTP 响应的数据（包括状态码和响应内容）
    void setResponseData(HTTP_STATUS_CODE, std::string);

    // 设置 Cookie，设置一些基本的 Cookie 参数
    void setCookie(const char* cookie, const char* path, int maxAge, bool httpOnly = true);

private:
    // 初始化 HTTP 响应头
    void initHttpResponseHead();

    // 在响应头的最后添加结束符（"\r\n"）
    void addHeaderEnd();

    // 设置连接属性（例如 Keep-Alive）
    void setConnection(Header* header);

    // 设置 Content-Length 响应头
    void setContentLength(Header* header);

    // 设置 Content-Type 响应头
    void setContentType(Header* header);

    // 设置 HTTP 响应头信息
    void setHeaderResponse(Header* header);

    // 添加 HTTP 响应头的 K-V 对
    void addHttpResponseHead(std::string k, std::string v);

    // 设置 Cookie 响应头
    void setCookie(Header* header);

    // 发送完整的 HTTP 响应头到客户端
    void sendResponseHeader(TcpClient* client);

private:
    // HTTP 响应头对象，用于存储响应头信息
    std::unique_ptr<ResponseHead> responseHead_;

    // 响应的主体内容数据
    std::string respData_;

    // Cookie 数据
    std::string cookie_;
};
