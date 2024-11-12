#include<vector>
#include<memory>
#include<thread>
#include<mutex>
#include"EventLoopThread.h"

class EventLoopThreadPool;
class EventLoop;

class EventLoopThreadPool{
public:
    EventLoopThreadPool();
    EventLoop* getNextLoop();
    ~EventLoopThreadPool();
    void start();
    void setThreadNum(int num);
    void createEventLoop();
private:
    using EventLoopThreadPtr = std::unique_ptr<EventLoopThread>;
    std::vector<EventLoopThreadPtr> eventLoopPtrPool_;
    std::vector<std::thread> threadPool_;
    size_t next_ = 0;
    int thread_num = 2;

    std::mutex mutex_;
};