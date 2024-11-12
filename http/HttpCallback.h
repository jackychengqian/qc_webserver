#pragma once  // 防止头文件的重复包含

#include "nocopyable.h"  // 包含不可复制基类
#include <functional>     // 引入 std::function，用于定义回调函数类型
#include <string>
#include <unordered_map>  // 引入 std::unordered_map，用于存储回调函数

class HttpInfo;  // 前置声明 HttpInfo 类，用于表示 HTTP 请求的信息

// 定义 HttpCallback 类，继承不可复制类 noncopyable，防止实例的复制和赋值
class HttpCallback : public noncopyable {
public:
    // 定义回调函数类型，使用 std::function 封装参数为 HttpInfo* 的函数
    using callback = std::function<void(HttpInfo*)>;

    // 设置 POST 请求的回调函数，参数 `s` 为 URI，`cb` 为回调函数
    static void setPostCB(const char* s, callback cb);

    // 获取指定 URI 的 POST 请求回调函数，返回值为 callback 类型
    static callback getPostCB(const char* s);

    // 设置 GET 请求的回调函数，参数 `s` 为 URI，`cb` 为回调函数
    static void setGetCB(const char* s, callback cb);

    // 获取指定 URI 的 GET 请求回调函数，返回值为 callback 类型
    static callback getGetCB(const char* s);

private:
    // 私有构造函数，禁止创建 HttpCallback 实例
    HttpCallback() = default;

    // 静态哈希表存储 POST 请求的回调函数，键为 URI，值为 callback
    static std::unordered_map<std::string, callback> postCallBack_;

    // 静态哈希表存储 GET 请求的回调函数，键为 URI，值为 callback
    static std::unordered_map<std::string, callback> getCallBack_;
};
