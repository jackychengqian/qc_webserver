#include "HttpParse.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "const.h"
#include "logger.h"
#include "Header.h"

#include <gflags/gflags.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unordered_set>

using namespace std;

// 定义 gflags 命令行参数
DEFINE_string(index, "hello.html", "web index html");        // 默认网页文件
DEFINE_string(path, "./www/dxgzg_src", "html path");         // 网页文件路径

// 简单过滤器集合，包含不允许的字符串
static unordered_set<string> filterSet{
    "http",
    "\\",
    "password",
    "/",
    "args",
    "bin",
    "user"
};

// 获取文件状态信息
bool RequestFileInfo::getFileStat() {
    fileFd_ = ::open(filePath_.c_str(), O_CLOEXEC | O_RDONLY);
    bool flag = true;

    if (fileFd_ < 0) {  // 未找到文件
        LOG_ERROR("file not exist: %s", filePath_.c_str());
        fileType_ = "html";
        filePath_ = FLAGS_path + "/404.html";
        fileFd_ = ::open(filePath_.c_str(), O_CLOEXEC | O_RDONLY);  // 打开 404 文件
        flag = false;
    }

    ::fstat(fileFd_, &fileStat_);
    fileSize_ = fileStat_.st_size;
    LOG_INFO("request file exist: %s", filePath_.c_str());
    return flag;
}

// 初始化 HTTP 响应头
void ResponseHead::initHttpResponseHead(HTTP_STATUS_CODE code) {
    switch (code) {
    case HTTP_STATUS_CODE::OK:
        responseStatue_ = "HTTP/1.1 200 OK\r\n";
        break;
    case HTTP_STATUS_CODE::NOT_FOUND:
        responseStatue_ = "HTTP/1.1 404 NOTFOUND\r\n";
        break;
    default:
        break;
    }
}

// 构造函数初始化成员
HttpInfo::HttpInfo() : response_(make_unique<HttpResponse>()),
                       request_(make_unique<HttpRequest>()),
                       parse_(make_unique<HttpParse>()) {}

HttpParse::HttpParse() : path_(FLAGS_path),
                         header_(make_unique<Header>()) {}

// 设置解析错误状态
void HttpParse::setError() {
    header_->status_ = PARSE_STATUS::PARSE_ERROR;
    header_->code_ = HTTP_STATUS_CODE::NOT_FOUND;
}

// 简单过滤器：检测请求 URI 中是否含有不允许的字符串
bool HttpParse::simpleFilter(string& s) {
    for (auto& val : filterSet) {
        if (s.find(val) != string::npos) return false;
    }
    return true;
}

// 解析请求行，包括方法、路径、参数等
bool HttpParse::parseRequestLine(const string& oneData, size_t oneIndex) {
    size_t index1 = oneData.find(" ");
    size_t index2 = oneData.find(" ", index1 + 1);

    if (index1 == string::npos || index2 == string::npos) {
        LOG_ERROR("http header error: %s", oneData.c_str());
        return false;
    }

    string method = oneData.substr(0, index1);
    bool ans = parseMethod(method);
    if (!ans) {
        LOG_ERROR("parse method error: %s", method.c_str());
        return false;
    }

    string path = oneData.substr(index1 + 1, index2 - index1 - 1);
    size_t flagSpilt = path.find("?");

    if (flagSpilt != string::npos) {
        header_->requestURI = path.substr(0, flagSpilt);
        string query = path.substr(flagSpilt + 1);

        size_t pos = 0;
        while (1) {
            size_t endIndex = query.find("&", pos);
            auto p = spilt(query, "=", pos, endIndex, 1);
            if (p.first == "" && p.second == "") break;
            header_->queryData_[p.first] = p.second;
        }
    } else {
        header_->requestURI = path;
    }

    return true;
}

// 解析 HTTP 头字段的键值对
void HttpParse::parseRequestKV(const string& request, size_t startIndex, size_t lastLineIndex) {
    size_t index = startIndex;
    size_t endIndex = 0;

    while (index < lastLineIndex && ((endIndex = request.find("\r\n", index)) != string::npos)) {
        auto p = spilt(request, ":", index, endIndex, 2);
        header_->kv_[p.first] = p.second;
    }
}

// 解析文件
bool HttpParse::analyseFile(const string& request) {
    if (header_->status_ == PARSE_STATUS::PARSE_NONE) {
        size_t oneIndex = request.find("\r\n");
        string oneData = request.substr(0, oneIndex);
        bool flag = parseRequestLine(oneData, oneIndex);
        if (!flag) {
            setError();
            return false;
        }

        size_t lastLineIndex = request.find("\r\n\r\n");
        if (lastLineIndex == string::npos) {
            setError();
            return false;
        }

        parseRequestKV(request, oneIndex + 2, lastLineIndex);

        flag = parseBody(request, lastLineIndex);
        if (!flag) {
            header_->status_ = PARSE_STATUS::PARSE_BODY_CONTINUE;
            return false;
        }

        setParseOK();
    } else if (header_->status_ == PARSE_STATUS::PARSE_BODY_CONTINUE) {
        size_t cLength = stoi(header_->kv_["Content-Length"]);
        if (header_->bodyTmp_.size() + request.size() == cLength) {
            setParseOK();
            header_->bodyData_ = move(header_->bodyTmp_) + move(request);
        } else {
            header_->bodyTmp_ += move(request);
            return false;
        }
    }

    if (header_->method_ == "GET") {
        bool flag = setResponseFile(header_->requestURI);
        if (!flag) {
            setError();
            return false;
        }
    }
    return true;
}

// 设置解析成功状态
void HttpParse::setParseOK() {
    header_->status_ = PARSE_STATUS::PARSE_OK;
    header_->code_ = HTTP_STATUS_CODE::OK;
}

// 解析请求体数据
bool HttpParse::parseBody(const string& request, size_t lastLineIndex) {
    auto it = header_->kv_.find("Content-Length");
    if (it != header_->kv_.end()) {
        size_t dataIndex = lastLineIndex + 4;
        string data = request.substr(dataIndex);

        size_t cLength = stoi(it->second);
        if (cLength != data.size()) {
            header_->bodyTmp_.reserve(cLength);
            header_->bodyTmp_ += std::move(data);
            return false;
        } else {
            header_->bodyData_ = move(data);
        }
    }

    return true;
}

// 设置请求文件信息
bool HttpParse::setResponseFile(string& requestFile) {
    if (requestFile == "/") {
        header_->reqFileInfo_->filePath_ = path_ + requestFile + FLAGS_index;
        requestFile = FLAGS_index;
    } else {
        header_->reqFileInfo_->filePath_ = path_ + requestFile;
    }
    LOG_INFO("request file path: %s", header_->reqFileInfo_->filePath_.c_str());

    bool flag = header_->reqFileInfo_->getFileStat();
    if (!flag) return false;

    flag = analyseFileType(header_->reqFileInfo_->filePath_);
    if (!flag) return false;

    return true;
}

// 解析文件类型
bool HttpParse::analyseFileType(const string& requestFile) {
    size_t i = requestFile.find_last_of(".");
    if (i == string::npos) {
        LOG_ERROR("parse file type error: %s", requestFile.c_str());
        return false;
    }
    header_->reqFileInfo_->fileType_ = requestFile.substr(i + 1);
    return true;
}

// 重置请求文件信息
void RequestFileInfo::reset() {
    fileFd_ = -1;
    fileName_ = "";
    filePath_ = "";
    fileSize_ = 0;
    fileType_ = "";
}

// 解析工具类重置
void HttpParse::reset() {
    header_->reqFileInfo_->reset();
    header_.reset(new Header());
}

// 检查解析是否完成
bool HttpInfo::isParseFinish() {
    return parse_->getHeader()->status_ != PARSE_STATUS::PARSE_BODY_CONTINUE &&
           parse_->getHeader()->status_ != PARSE_STATUS::PARSE_HEADER_CONTINUE;
}

// 获取请求体数据
string HttpInfo::getBodyData() {
    return parse_->getHeader()->bodyData_;
}

// 设置响应数据
void HttpInfo::setResponse(HTTP_STATUS_CODE code, const char* data) {
    response_->setResponseData(code, data);
}

// 默认析构函数
RequestFileInfo::~RequestFileInfo() = default;
HttpParse::~HttpParse() = default;
HttpInfo::~HttpInfo() = default;
ResponseHead::~ResponseHead() = default;
