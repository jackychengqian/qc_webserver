### 1. **初始化 HTTP 响应头**
   - 在构造函数 `HttpResponse::HttpResponse()` 中初始化响应头和响应体的基本内容。响应头的初始化细节在 `initHttpResponseHead()` 函数中实现。
   - `initHttpResponseHead()` 函数中设置了 `Server` 和 `Date` 头部字段：
     - `Server` 字段值通过 `gflags` 由 `FLAGS_serverName` 决定，默认值为 `"Pine"`。
     - `Date` 字段由 `TimeStamp::getUTC()` 提供当前的 UTC 时间。

### 2. **发送响应头**
   - `sendResponseHeader()` 函数用于将 HTTP 响应头发送至客户端 `TcpClient`。
     - 如果 `respData_`（响应体）不为空，则会将其内容附加到 `responseHeader_` 中。
     - 最终将响应头 `responseStatue_` 和 `responseHeader_` 组合后，作为完整的响应头通过 `client->send()` 发送到客户端。
     - 发送前会记录完整的响应头内容供调试或日志记录使用。

### 3. **发送文件响应**
   - `SendFile()` 函数是核心文件响应流程，分为以下几步：
     - **检查解析完成状态**：调用 `httpInfo->isParseFinish()` 检查请求解析状态，如果未完成，则直接返回。
     - **处理 POST 请求**：
       - POST 请求时，调用 `HttpCallback::getPostCB()` 通过 `requestURI` 查找是否有对应的回调函数。
       - 若找到对应的回调函数 `cb`，则执行该回调，以处理 POST 请求的数据。
     - **处理 GET 请求**：
       - GET 请求时，设置响应头的状态码，通过 `responseHead_->initHttpResponseHead(header->code_)` 初始化状态码。
     - **设置响应头**：通过 `setHeaderResponse(header.get())` 将请求头中的 `Connection`、`Content-Length`、`Content-Type`、`Set-Cookie` 等设置到响应头。
     - **发送响应头**：调用 `sendResponseHeader()` 发送包含状态行和各项头部的响应头。

### 4. **读取并发送文件内容**
   - `SendFile()` 函数继续处理文件发送部分：
     - **开始计时**：记录发送开始时间（`client->start`）。
     - **读取文件内容**：
       - 根据请求文件信息 `reqFileInfo` 中的 `fileFd_`（文件描述符）和 `fileSize_`（文件大小）读取文件内容。
       - 使用 `malloc` 分配缓冲区，将文件内容读取到 `buff` 中，然后创建字符串 `s` 保存文件内容。
     - **发送文件内容**：
       - 使用 `client->send()` 将文件内容字符串 `s` 发送给客户端。
     - **清理资源**：
       - 释放缓冲区 `buff` 并关闭文件描述符 `reqFileInfo->fileFd_`。

### 5. **管理连接状态**
   - 在 `SendFile()` 中判断是否需要关闭连接：
     - 检查响应头是否包含 `Connection` 字段，或其值是否为 `"close"`，若是则关闭连接。
     - 通过 `client->CloseCallback()` 执行关闭连接回调函数。

### 6. **设置响应头的细节**
   - `setHeaderResponse()` 函数用于初始化并添加各个响应头字段，调用顺序如下：
     - **初始化基本响应头**：通过 `initHttpResponseHead()` 设置 `Server` 和 `Date`。
     - **设置 Connection**：在 `setConnection()` 中检查请求头中 `Connection` 字段的值，若为 `"keep-alive"` 则保持连接。
     - **设置 Content-Length**：`setContentLength()` 中根据请求类型设置长度字段：
       - 对于 GET 请求，`Content-Length` 设置为文件大小。
       - 对于 POST 请求，`Content-Length` 设置为 `respData_` 的长度。
     - **设置 Content-Type**：在 `setContentType()` 中：
       - GET 请求时根据文件扩展名确定类型（如 `.jpg`、`.css` 等），利用 `httpContentTypes` 查找对应的 MIME 类型。
       - POST 请求默认类型为 `application/json`，两种类型均追加字符集信息 `"charset=utf-8"`。
     - **设置 Set-Cookie**：在 `setCookie()` 中，若 `cookie_` 非空，则添加 `Set-Cookie` 头部字段。

### 7. **添加响应头中的键值对**
   - `addHttpResponseHead(string k, string v)` 将键值对添加到 `responseHeader_` 中，调用 `addHeaderEnd()` 在每个头部后加上 `\r\n` 结束符。

### 8. **设置响应体内容**
   - `setResponseData()` 函数根据给定的状态码和数据内容设置响应体数据，将状态码初始化到响应头中，同时将数据追加到 `respData_`。

### 9. **析构函数**
   - `HttpResponse::~HttpResponse()` 用于释放资源，确保对象生命周期结束后不再持有资源。

