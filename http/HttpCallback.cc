#include "HttpCallback.h"  // 包含 HttpCallback 类定义
#include "HttpParse.h"      // 包含 HTTP 解析相关定义
using callback = HttpCallback::callback;  // 定义别名，简化 HttpCallback::callback 类型的使用

// 设置 POST 请求的回调函数，键为 URI，值为回调函数
void HttpCallback::setPostCB(const char* s, callback cb) {
    postCallBack_[s] = std::move(cb);  // 将回调函数插入 postCallBack_，使用 std::move 进行高效移动
}

// 获取指定 URI 的 POST 请求回调函数
callback HttpCallback::getPostCB(const char* s) {
    // 如果找不到指定 URI 的回调函数，返回 nullptr
    if (postCallBack_.find(s) == postCallBack_.end()) {
        return nullptr;
    }
    return postCallBack_[s];  // 返回找到的回调函数
}

// 设置 GET 请求的回调函数，键为 URI，值为回调函数
void HttpCallback::setGetCB(const char* s, callback cb) {
    getCallBack_[s] = std::move(cb);  // 将回调函数插入 getCallBack_，使用 std::move 进行高效移动
}

// 获取指定 URI 的 GET 请求回调函数
callback HttpCallback::getGetCB(const char* s) {
    // 如果找不到指定 URI 的回调函数，返回 nullptr
    if (getCallBack_.find(s) == getCallBack_.end()) {
        return nullptr;
    }
    return getCallBack_[s];  // 返回找到的回调函数
}

// 静态成员变量初始化
std::unordered_map<std::string, HttpCallback::callback> HttpCallback::postCallBack_ = 
    std::unordered_map<std::string, HttpCallback::callback>();

std::unordered_map<std::string, HttpCallback::callback> HttpCallback::getCallBack_ = 
    std::unordered_map<std::string, HttpCallback::callback>();
