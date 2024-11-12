#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <thread>   
#include <mutex>
#include "EPoller.h"
#include "const.h"

class Channel;
class HeartConnect;
class TimerQueue;

class EventLoop{
    using Function = std::function<void()>;
public:
    EventLoop();
    void loop();
    void updateChannel(Channel *channel);
    void runInLoop(const Function &cb);

    void queueInLoop(const Function& cb);
    void wakeup();
    void handleRead();
    void doPendingFunctor();

    std::thread::id getThreadId()const{return threadId_;};
    void addTimer(timerCallback cb,int expire,bool repeat = false);

    ~EventLoop();
private:
    std::unique_ptr<EPoller> epoller_;
    int wakefd_;
    std::vector<Function> pendingFunctors_;
    std::thread::id threadId_;
    mutable std::mutex mutex_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::unique_ptr<TimerQueue> timerQueue_;
};