#pragma once  // 防止重复包含

#include <map>
#include <string>
#include <memory>
#include "const.h"  // 包含与 HTTP 请求和解析相关的常量和枚举

// 前置声明，定义结构体类型 RequestFileInfo，减少编译依赖
struct RequestFileInfo;

// 定义 Header 类，用于存储和管理 HTTP 请求的头部信息
struct Header {
public:
    std::map<std::string, std::string> kv_;           // 存储 HTTP 请求头部字段的键值对
    std::map<std::string, std::string> queryData_;    // 存储查询字符串的键值对
    std::string bodyData_;                            // 存储请求体数据
    std::string headerTmp_;                           // 临时存储完整的请求头部数据
    std::string bodyTmp_;                             // 临时存储完整的请求体数据
    std::string requestURI;                           // 存储请求的 URI
    std::unique_ptr<RequestFileInfo> reqFileInfo_;    // 指向请求文件信息的智能指针
    std::string method_;                              // 存储 HTTP 请求方法（如 GET、POST）
    PARSE_STATUS status_;                             // 存储解析状态，类型为解析状态枚举
    HTTP_STATUS_CODE code_;                           // 存储 HTTP 响应状态码

    Header();        // 构造函数，初始化各成员
    ~Header() = default;  // 默认析构函数
};
