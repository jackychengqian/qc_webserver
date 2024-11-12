#include "HttpRequest.h"
#include "TcpClient.h"
#include "logger.h"
#include "HttpParse.h"
#include "HttpResponse.h"

using namespace std;

// HttpRequest 类的成员函数，用于分析 HTTP 请求
bool HttpRequest::analyseFile(std::string& msg, unique_ptr<HttpInfo>& httpInfo) {
    // 调用 httpInfo 中 HttpParse 的 analyseFile 函数解析请求消息 msg
    return httpInfo->parse_->analyseFile(msg);
}

// HttpRequest 类的析构函数
HttpRequest::~HttpRequest() = default;
