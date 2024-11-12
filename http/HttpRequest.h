#pragma once 
#include <string>
#include <memory>
#include "HttpResponse.h"
#include "nocopyable.h"  // 包含不可复制的类定义

// HttpRequest 类用于解析 HTTP 请求，并生成相应的 HttpInfo 信息
class HttpRequest : public noncopyable  // 继承自不可复制的基类 noncopyable
{
public:
    // 默认构造函数
    HttpRequest() = default;
    // 析构函数，释放资源
    ~HttpRequest();

    // 分析 HTTP 请求消息，填充 HttpInfo 对象
    bool analyseFile(std::string& msg, std::unique_ptr<HttpInfo>& httpInfo);
};
