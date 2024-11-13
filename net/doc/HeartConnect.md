## HeartConnect 模块工作过程

1. **模块初始化**

   - 服务器启动时，`HeartConnect` 类被实例化并初始化。
   - 构造函数 `HeartConnect()` 创建了一个定时器文件描述符 `timefd_`，用于周期性触发心跳检查。
   - `timefd_` 被绑定到 `channel_` 对象上，并设置了读事件回调 `headRead()`。
   - 设置定时器的超时时间 `expire_`，定时器将在每 `expire_` 秒触发一次 `headRead()`，从而定期检查所有连接的活动状态。

2. **客户端连接的添加**

   - 当一个新的客户端连接建立时，`HeartConnect::add()` 被调用，将该连接添加到 `HeartConnect` 管理的心跳检测中。
   - `add()` 首先检查该客户端是否已在 `fdMap_` 中：
     - **如果不存在**：表示这是一个新的连接。`add()` 创建一个新的 `Node` 对象，其中包含客户端的文件描述符（`fd_`）、`TcpClient` 智能指针（`cPtr_`）、当前时间戳（`time_`）以及关闭连接的回调函数（`callback_`）。然后将该节点插入 `NodeList_` 的头部（代表最新活动的时间），并更新 `fdMap_` 和 `NodeMap_`。
     - **如果已存在**：表示该连接刚刚活动过，调用 `adjust(fd)` 调整连接的超时时间。

3. **调整连接的超时时间**

   - 当已有客户端再次活动时，`adjust(fd)` 将更新其心跳时间：
     - `adjust(fd)` 通过 `fdMap_` 查找客户端的 `Node` 指针，将其时间戳更新为当前时间。
     - 将该节点从 `NodeList_` 的原位置移除，并重新插入到头部，表明该连接为最新活动状态。
     - `NodeMap_` 中也同步更新其在 `NodeList_` 中的位置。

4. **心跳检测（定时触发）**

   - 定时器每隔 `expire_` 秒触发一次，执行 `headRead()`：
     - `headRead()` 会读取 `timefd_` 的定时器事件，清除触发事件以便再次触发。
     - `headRead()` 获取当前时间，然后从 `NodeList_` 尾部开始遍历，检查节点的时间戳：
       - **如果某个节点超时**（即：`time < now.tv_sec`），则调用 `del(ptr)` 删除该节点。
       - **如果某个节点未超时**，则停止遍历，因为 `NodeList_` 是按时间顺序排列的。

5. **超时处理**

   - `del(NodePtr ptr)` 负责处理超时的客户端连接：
     - 首先检查 `ptr->cPtr_` 是否存在。如果存在，将客户端连接状态设为 `MUST_CLOSE_CONNECT`，并调用 `callback_` 执行关闭逻辑。
     - 如果 `ptr->cPtr_` 不存在，表明连接已被提前释放，调用 `destroyConnect()` 清理所有数据。

6. **连接的清理**

   - **主动关闭**：当客户端主动断开连接时，服务器会调用 `clientCloseCallback(int fd)`，该函数会查找文件描述符对应的节点并调用 `destroyConnect()` 进行清理。
   - **被动清理**：在 `del()` 中若发现超时节点的 `TcpClient` 不存在，也会调用 `destroyConnect()` 清理资源。

7. **资源销毁**

   - `destroyConnect(NodePtr ptr)` 将节点从 `NodeList_`、`fdMap_`、`NodeMap_` 中移除，完全销毁该连接的所有相关数据，确保不会再触发超时检查。

---

## 工作示例

- **场景**：客户端 A 建立连接，保持活动一段时间后无响应，导致超时。
   - 服务器接收到客户端 A 的连接请求，调用 `add()` 添加到 `HeartConnect`，并初始化心跳时间。
   - 若客户端 A 定期发送数据，服务器每次都会调用 `adjust()` 更新其心跳时间。
   - 一段时间后，客户端 A 停止发送数据，`expire_` 秒后定时器触发 `headRead()` 检查，发现 A 超时，调用 `del()` 执行 `callback_` 并清理其数据。

---

这个流程保证了 `HeartConnect` 能及时检测和处理客户端的活动状态，并在超时或主动关闭的情况下释放资源，从而维持高效和安全的服务器性能。