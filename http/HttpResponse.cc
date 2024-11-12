#include "HttpResponse.h"
#include "HttpParse.h"
#include "Header.h"
#include "HttpRequest.h"
#include "TimeStamp.h"
#include "HttpCallback.h"
#include <gflags/gflags.h>

#if 1
#include <chrono>
using namespace std::chrono;
using namespace std;
#endif

using namespace rapidjson;

// 通过 gflags 定义了服务器名称（默认 "Pine"）
DEFINE_string(serverName, "Pine", "server name");

// HTTP 响应头中的 Content-Type 映射表
std::map<std::string, std::string> httpContentTypes = {
    {"js", "application/x-javascript"},
    {"css", "text/css"},
    {"png", "image/png"},
    {"jpg", "image/jpg"},
    {"tar", "application/tar"},
    {"zip", "application/zip"},
    {"html", "text/html"},
    {"json", "application/json"},
    {"jpeg", "image/jpeg"},
    {"woff2", "font/woff2"}
};

// 初始化 HTTP 响应头
void HttpResponse::initHttpResponseHead() {
    // 添加 "Server" 和 "Date" 响应头
    addHttpResponseHead("Server", FLAGS_serverName);
    addHttpResponseHead("Date", TimeStamp::getUTC());
}

// 构造函数：初始化 HttpResponse 对象，分配内存给 responseHead_
HttpResponse::HttpResponse() : responseHead_(make_unique<ResponseHead>()), respData_(""), cookie_("") {}

// 发送 HTTP 响应头到客户端
void HttpResponse::sendResponseHeader(TcpClient* client) {
    // 如果响应体不为空，附加到响应头
    if (!respData_.empty()) {
        responseHead_->responseHeader_ += respData_;
    }
    // 将响应头的状态部分组合起来
    responseHead_->responseStatue_ += responseHead_->responseHeader_;
    // 记录响应头信息
    LOG_INFO("send response http header:%s", responseHead_->responseStatue_.c_str());
    // 发送响应头到客户端
    client->send(responseHead_->responseStatue_);
}

// 设置 Cookie 响应头
void HttpResponse::setCookie(const char* cookie, const char* path, int maxAge, bool httpOnly) {
    cookie_ += ";";
    cookie_ += path;
    cookie_ += ";";
    cookie_ += to_string(maxAge);
    // 如果设置 HttpOnly，附加到 cookie 字符串中
    if (httpOnly) {
        cookie_ += ";";
        cookie_ += "HttpOnly";
    }
}

// 发送文件响应给客户端
void HttpResponse::SendFile(TcpClient* client, std::unique_ptr<HttpInfo>& httpInfo) {
    // 如果解析没有完成，则直接返回
    if (!httpInfo->isParseFinish()) return;

    auto& header = httpInfo->parse_->getHeader();
    
    // 处理 POST 请求
    if (header->method_ == "POST") {
        auto cb = HttpCallback::getPostCB(header->requestURI.c_str());
        if (cb == nullptr) {
            LOG_ERROR("%s not callback fun", header->requestURI.c_str());
        } else {
            cb(httpInfo.get());
        }
    } 
    // 处理 GET 请求
    else if (header->method_ == "GET") {
        responseHead_->initHttpResponseHead(header->code_);
    }
    
    // 设置响应头（包括 Connection, Content-Length, Content-Type 等）
    setHeaderResponse(header.get());

    // 发送响应头给客户端
    sendResponseHeader(client);

    // 获取请求文件信息
    auto& reqFileInfo = header->reqFileInfo_;
    
    // 开始计时
    client->start = system_clock::now();
    
    // 读取文件到缓冲区
    char* buff = (char*)malloc(reqFileInfo->fileSize_);
    ::read(reqFileInfo->fileFd_, buff, reqFileInfo->fileSize_);
    // 创建文件内容的字符串
    string s(buff, reqFileInfo->fileSize_);
    
    // 发送文件内容到客户端
    int n = client->send(std::move(s));
    
    // 释放缓冲区
    free(buff);
    // 关闭文件描述符
    close(reqFileInfo->fileFd_);

    // 如果响应头中没有设置 Connection 或者设置为 "close"，则关闭连接
    if (n == -1 || header->kv_.find("Connection") == header->kv_.end() || header->kv_["Connection"].find("close") != string::npos) {
        client->CloseCallback();
    }
}

// 向响应头添加一个键值对
void HttpResponse::addHttpResponseHead(string k, string v) {
    responseHead_->responseHeader_ += k + ":" + v;
    addHeaderEnd();
}

// 设置响应体数据
void HttpResponse::setResponseData(HTTP_STATUS_CODE code, std::string data) {
    // 设置响应头，并初始化响应体
    responseHead_->initHttpResponseHead(code);
    respData_ += data;
}

// 添加响应头的结束符 "\r\n"
void HttpResponse::addHeaderEnd() {
    string s = "\r\n";
    responseHead_->responseHeader_ += s;
}

// 设置 "Connection" 响应头，如果需要保持连接
void HttpResponse::setConnection(Header* header) {
    // 如果响应头中没有设置 "Connection" 或设置为 "close"，则不做处理
    if (header->kv_.find("Connection") != header->kv_.end() && header->kv_["Connection"].find("close") == string::npos
        && header->code_ != HTTP_STATUS_CODE::NOT_FOUND) {
        // 设置为 "keep-alive" 保持连接
        addHttpResponseHead("Connection", "keep-alive");
    }
}

// 设置 "Content-Length" 响应头
void HttpResponse::setContentLength(Header* header) {
    string key = "Content-Length";
    string value = "";

    // 对 GET 请求，设置为文件的大小
    if (header->method_ == "GET") {
        value = to_string(header->reqFileInfo_->fileSize_);
    } 
    // 对 POST 请求，设置为响应数据的大小
    else if (header->method_ == "POST") {
        value = to_string(respData_.size());
    }
    addHttpResponseHead(key, value);
}

// 设置 "Content-Type" 响应头
void HttpResponse::setContentType(Header* header) {
    string key = "Content-Type";
    string value = "";

    // 对 GET 请求，设置为文件类型
    if (header->method_ == "GET") {
        value = httpContentTypes[header->reqFileInfo_->fileType_];
    } 
    // 对 POST 请求，默认设置为 "application/json"
    else if (header->method_ == "POST") {
        value = httpContentTypes["json"];
    }
    // 添加字符集信息
    value += "; charset=utf-8"; 
    addHttpResponseHead(key, value);
}

// 设置 Cookie 响应头
void HttpResponse::setCookie(Header* header) {
    if (cookie_.empty()) return;
    string k = "Set-Cookie";
    addHttpResponseHead(k, cookie_);
}

// 设置所有响应头
void HttpResponse::setHeaderResponse(Header* header) {
    // 初始化响应头
    initHttpResponseHead();
    // 设置 "Connection", "Content-Length", "Content-Type", "Set-Cookie" 等
    setConnection(header);
    setContentLength(header);
    setContentType(header);
    setCookie(header);
    addHeaderEnd();
}

// 析构函数：释放资源
HttpResponse::~HttpResponse() {}
