#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "logger.h"

#include <sstream>
#include <atomic>
using namespace std;

atomic<int> h_i;
EventLoopThreadPool::EventLoopThreadPool() = default;

void EventLoopThreadPool::setThreadNum(int num){
    LOG_INFO("set thread num to %d", num);
    this->thread_num = num;
}

void EventLoopThreadPool::start(){
    for(int i = 0; i < thread_num; i++){
        std::thread t(&EventLoopThreadPool::createEventLoop,this);
        std::ostringstream oss;
        oss << t.get_id();
        threadPool_.push_back(std::move(t));
    }
}

EventLoop* EventLoopThreadPool::getNextLoop(){
    if(next_ == eventLoopPtrPool_.size()){
        next_ = 0;
    }
    LOG_INFO("线程个数:%lu",eventLoopPtrPool_.size());
    if(eventLoopPtrPool_[next_]->getLoop() ==nullptr){
        LOG_FATAL("loop is null");
    }
    
    return eventLoopPtrPool_[next_++]->getLoop();
}

EventLoopThreadPool::~EventLoopThreadPool(){
    for(auto &t:threadPool_){
        t.detach();
    }
}

void EventLoopThreadPool::createEventLoop(){
    LOG_INFO("create new thread threadid:%s",oss.str().c_str());
    EventLoopThreadPtr ptr = std::make_unique<EventLoopThread>();
    EventLoop* loop = ptr->getLoop();
    std::unique_lock<std::mutex> lock(mutex_);
    eventLoopPtrPool_.push_back(std::move(ptr));
    lock.unlock();
    loop->loop();
}