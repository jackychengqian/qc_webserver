#pragma once  // 防止头文件重复包含

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>
#include <gflags/gflags.h>
#include <unordered_map>

#include "TcpClient.h"  // TCP 客户端相关头文件
#include "logger.h"     // 日志管理相关头文件
#include "const.h"      // 常量和枚举定义

class HttpResponse;
class HttpRequest;
class Header;  // 预声明，减少依赖

// 存储请求的文件信息
struct RequestFileInfo {
    int fileFd_ = -1;                  // 文件描述符
    std::string filePath_ = "";        // 文件路径
    std::string fileType_ = "";        // 文件类型
    std::string fileName_ = "";        // 文件名
    size_t fileSize_ = 0;              // 文件大小
    struct stat fileStat_;             // 文件状态结构体

public:
    RequestFileInfo() : fileFd_(-1), filePath_(""), fileType_(""), fileName_(""), fileSize_(0), fileStat_() {}

    // 获取文件状态信息
    bool getFileStat();

    // 重置文件信息
    void reset();

    // 析构函数
    ~RequestFileInfo();
};

// 表示 HTTP 响应头信息
struct ResponseHead {
    std::string responseHeader_;       // HTTP 响应头
    std::string responseStatue_;       // 响应状态行
    std::string serverName_;           // 服务器名称

public:
    ResponseHead() = default;

    // 初始化 HTTP 响应头信息
    void initHttpResponseHead(HTTP_STATUS_CODE code);

    // 析构函数
    ~ResponseHead();
};

// 解析 HTTP 请求的类
class HttpParse {
private:
    std::string path_;                 // 请求路径

    std::unique_ptr<Header> header_;   // 存储解析的 HTTP 头部信息

public:
    HttpParse();

    // 分析文件信息
    bool analyseFile(const std::string&);

    // 设置响应文件信息
    bool setResponseFile(std::string&);

    // 分析文件类型
    bool analyseFileType(const std::string&);

    // 分割字符串，用于解析键值对
    std::pair<std::string, std::string> spilt(const std::string& s, std::string sep, size_t& pos, size_t endIndex, size_t addPos);

    // 重置解析状态
    void reset();

    // 获取 HTTP 头部信息
    std::unique_ptr<Header>& getHeader() { return header_; }

    // 析构函数
    ~HttpParse();

private:
    // 解析请求行（包含方法、路径和版本）
    bool parseRequestLine(const std::string&, size_t);

    // 判断请求方法是否有效
    bool parseMethod(std::string&);

    // 设置错误状态码
    void setError();

    // 解析 HTTP 头部字段的键值对
    void parseRequestKV(const std::string&, size_t, size_t);

    // 解析请求体数据
    bool parseBody(const std::string&, size_t);

    // 设置解析完成状态
    void setParseOK();

    // 简单过滤器
    bool simpleFilter(std::string&);
};

// 存储和管理 HTTP 请求
struct HttpInfo {
    std::unique_ptr<HttpResponse> response_;  // 响应信息
    std::unique_ptr<HttpRequest> request_;    // 请求信息
    std::unique_ptr<HttpParse> parse_;        // 解析信息

    HttpInfo();

    // 析构函数
    ~HttpInfo();

    // 重置 HTTP 信息
    void reset();

    // 检查解析是否完成
    bool isParseFinish();

    // 获取请求体数据
    std::string getBodyData();

    // 设置响应状态码和消息
    void setResponse(HTTP_STATUS_CODE, const char*);
};
