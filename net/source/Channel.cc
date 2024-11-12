#include "Channel.h"
#include "EventLoop.h"

#include<sys/epoll.h>

Channel::Channel(EventLoop* loop,int fd):loop_(loop),fd_(fd){}

void Channel::handleEvent(){
    if(!(curEvents_&EPOLLIN) && (curEvents_&EPOLLHUP) && closeCallback_){
        closeCallback_();
    }
    else if(curEvents_ & EPOLLIN || curEvents_ & EPOLLPRI){
        // cout<<"readCallback()_"<<endl;
        readCallback_();
    }
    else if(curEvents_ & EPOLLOUT){
        writeCallback_();
    }
}

void Channel::update(){
    loop_->updateChannel(this);
}