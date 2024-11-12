#pragma once
#include<memory>

#include"logger.h"

class EventLoop;

class EventLoopThread{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* getLoop()const{
        if(loop_.get() == nullptr){
            LOG_FATAL("loop_ is nullptr");
        }
        return loop_.get();
    }   
private:
    std::unique_ptr<EventLoop> loop_;
};