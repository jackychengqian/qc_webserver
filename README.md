
# 模块详细介绍

## 1. HttpServer 模块

`HttpServer` 类是整个服务器的核心，负责启动服务器、监听客户端连接、并分发请求。

### 主要功能

- **管理 TCP 连接**：处理客户端的连接请求和数据读取。
- **多线程支持**：通过配置工作线程池处理多客户端并发连接，提升性能。
- **请求解析和响应生成**：接受 HTTP 请求并将解析后的内容传递给 `HttpResponse` 生成相应的响应数据。

### 关键方法

- **`run()`**：启动 HTTP 服务器的事件循环。该方法初始化线程池（线程数可通过 `--threadNum` 配置），并启动主事件循环 `loop_` 以便持续监听端口。
- **`ReadCallback(Pine::clientPtr t, Buffer* inputBuffer)`**：这是数据读取回调函数。每次客户端请求到达时调用，读取请求内容并交给 `HttpRequest` 解析，再由 `HttpResponse` 生成并返回响应。
- **`setPostReadCallback(const char* s, callback cb)`**：用于自定义处理 POST 请求的回调。支持用户自定义响应内容和逻辑。

### 数据成员

- **`EventLoop loop_`**：事件循环对象，负责监听客户端请求和处理事件。
- **`TcpServer tcpServer_`**：封装了底层的 TCP 服务器，提供基本的 TCP 通信功能。
- **`callback postCallback_`**：POST 请求的自定义回调函数指针，用于处理特定路径的 POST 请求。

---

## 2. HttpRequest 模块

`HttpRequest` 模块主要负责解析 HTTP 请求，提取请求头、请求方法、URL 路径等关键信息，支持 GET 和 POST 方法。

### 主要功能

- **解析请求头**：分解请求行和请求头，提取请求方法、URI、协议版本等信息。
- **处理请求体**：在 POST 请求中处理请求数据并支持 JSON 解析。

### 关键方法

- **`parseRequest(const std::string& rawRequest)`**：解析原始的 HTTP 请求字符串，分解出请求头和请求体。根据请求类型的不同，调用不同的解析逻辑。
- **`analyseFile(const std::string& path, std::unique_ptr<HttpInfo>& httpInfo)`**：检查并确定请求的资源文件是否存在，若存在则在 `httpInfo` 中记录该文件路径及类型（例如静态文件、HTML 页面等）。
- **`getMethod()`、`getURI()`**：分别返回解析出的 HTTP 请求方法和 URI 路径。

### 数据成员

- **`std::string method_`**：存储请求方法（如 GET 或 POST）。
- **`std::string uri_`**：存储请求的 URI 路径。
- **`std::unordered_map<std::string, std::string> headers_`**：存储 HTTP 请求头字段及其对应值。

---

## 3. HttpResponse 模块

`HttpResponse` 模块负责生成 HTTP 响应，包括响应头和响应体。该模块支持设置不同的状态码、内容类型、以及 Cookie 等。

### 主要功能

- **生成响应头**：设置状态码、内容类型和 Cookie。
- **文件响应**：根据请求的路径，加载对应的文件内容并返回。
- **JSON 响应**：支持 JSON 格式的数据返回，方便客户端解析。

### 关键方法

- **`setStatusCode(int code)`**：设置 HTTP 状态码（例如 200、404）。
- **`setResponseData(const std::string& data)`**：直接设置响应体内容，通常用于返回 JSON 数据或自定义文本。
- **`SendFile(TcpClient* client, std::unique_ptr<HttpInfo>& httpInfo)`**：根据 `httpInfo` 中的文件路径读取文件内容，并将其发送给客户端。
- **`setCookie(const char* cookie, const char* path, int maxAge, bool httpOnly)`**：设置响应的 Cookie。支持 `path`、`maxAge` 和 `HttpOnly` 属性。

### 数据成员

- **`std::string status_`**：存储 HTTP 响应状态码和状态描述。
- **`std::string body_`**：存储响应体内容。
- **`std::unordered_map<std::string, std::string> headers_`**：存储 HTTP 响应头信息。
- **`std::string cookie_`**：存储 HTTP 响应的 Cookie 内容。

---

## 4. Cookie 模块

`Cookie` 模块用于管理 HTTP 响应中的 Cookie。该模块提供基础的 Cookie 设置和格式化功能。

### 主要功能

- **添加和格式化 Cookie**：通过设置属性（例如路径、有效期等）生成格式化的 Cookie 字符串，以便加入到响应头中。
- **管理 Cookie 属性**：设置 `path`、`maxAge`、`HttpOnly` 等属性，实现简单的会话管理功能。

### 关键方法

- **`addCookie(const std::string& name, const std::string& value, int maxAge = 0, bool httpOnly = false)`**：添加单个 Cookie，指定 Cookie 的名称、值及其他属性。
- **`getCookieHeader()`**：返回完整的、格式化后的 Cookie 头字符串，用于加入到 HTTP 响应头中。

### 数据成员

- **`std::unordered_map<std::string, std::string> cookies_`**：存储所有的 Cookie 键值对。
- **`std::string cookieHeader_`**：格式化的 Cookie 头信息字符串。

---

## 5. TcpClient 模块

`TcpClient` 模块负责管理单个客户端连接的 TCP 通信。该模块封装了 TCP 套接字的基本操作，为 `HttpServer` 提供底层网络支持。

### 主要功能

- **数据接收和发送**：封装了数据接收、发送的操作，支持读写缓冲区的管理。
- **异常处理**：检测和处理客户端断开连接、读取异常等网络异常情况。

### 关键方法

- **`sendData(const std::string& data)`**：将指定数据写入发送缓冲区并发送给客户端。
- **`receiveData(Buffer* buffer)`**：从客户端套接字读取数据，存入 `Buffer` 缓冲区中，供 `HttpServer` 解析。
- **`closeConnection()`**：关闭客户端连接，释放相关资源。

### 数据成员

- **`int sockfd_`**：客户端的套接字文件描述符。
- **`Buffer readBuffer_`**：存储客户端请求数据的缓冲区。
- **`Buffer writeBuffer_`**：存储服务器响应数据的缓冲区。

---

## 6. Buffer 模块

`Buffer` 模块是 TCP 通信的辅助模块，用于管理读写数据的缓冲区。该模块简化了网络数据的处理，支持对请求和响应数据的缓存。

### 主要功能

- **数据缓存**：缓冲来自 TCP 客户端的请求数据和服务器的响应数据。
- **字符串获取**：提供从缓冲区中提取字符串的操作，方便 HTTP 请求和响应的处理。

### 关键方法

- **`append(const char* data, size_t len)`**：将指定数据添加到缓冲区尾部。
- **`getAllString()`**：提取缓冲区中的所有数据，并返回为字符串格式。
- **`retrieve(size_t len)`**：从缓冲区中移除指定长度的数据，调整读指针位置。

### 数据成员

- **`std::vector<char> buffer_`**：实际存储数据的缓冲区。
- **`size_t readPos_`**：读指针位置。
- **`size_t writePos_`**：写指针位置。

---

## 7. Logger 模块

`Logger` 模块负责服务器的日志记录，包括请求、错误和调试信息的输出。通过该模块，可以轻松定位服务器在运行中出现的问题。

### 主要功能

- **信息记录**：将服务器运行过程中的关键操作记录到日志文件中。
- **级别控制**：支持不同的日志级别（如 INFO、WARN、ERROR），便于调试和定位问题。

### 关键方法

- **`LOG_INFO(const char* format, ...)`**：记录信息级别的日志。
- **`LOG_WARN(const char* format, ...)`**：记录警告级别的日志。
- **`LOG_ERROR(const char* format, ...)`**：记录错误级别的日志。

### 数据成员

- **`std::ofstream logFile_`**：用于记录日志的文件流。
- **`LogLevel level_`**：当前日志级别。

---

