#include "HeartConnect.h"
#include "Channel.h"
#include "EventLoop.h"
#include "logger.h"
#include "TcpClient.h"

#include <sys/timerfd.h>
#include <assert.h>
#include <unistd.h>

static bool isSameThread(const std::thread::id& thread_id){
    return thread_id == std::this_thread::get_id();
}

static int createTimeFd(){
    return timerfd_create(CLOCK_REALTIME,TFD_NONBLOCK);
}

static __time_t getNow(){
    timespec now;
    int ret = clock_gettime(CLOCK_REALTIME,&now);
    assert(ret != -1);
    return now.tv_sec;
}

HeartConnect::HeartConnect(EventLoop *loop,int expire):
    NodeList_(),NodeMap_(),fdMap_(),timefd_(createTimeFd()),
    expire_(expire),loop_(loop),channel_(std::make_unique<Channel>(loop_,timefd_))
{
    assert(timefd_ != -1);
    channel_->setReadCallback(std::bind(&HeartConnect::headRead,this));
    channel_->enableReadEvent();
    
    itimerspec new_value;
    new_value.it_value.tv_sec = expire_;
    new_value.it_value.tv_nsec = 0;

    new_value.it_interval.tv_sec = expire_;
    new_value.it_interval.tv_nsec = 0;
    
    int ret = timerfd_settime(timefd_, 0, &new_value, NULL);//启动定时器
    assert(ret != -1);
}

void HeartConnect::add(int fd,std::shared_ptr<TcpClient> cPtr,std::function<void()> callback)
{
    assert(isSameThread(loop_->getThreadId()));
    if(fdMap_.find(fd) == fdMap_.end()){
        LOG_INFO("add new headContent:%d",fd);
        NodePtr ptr = std::make_shared<Node>();
        ptr->fd_ = fd;
        ptr->cPtr_ = cPtr;
        ptr->time_ = getNow();
        ptr->callback_ = std::move(callback);
        fdMap_.emplace(fd,ptr);
        NodeList_.emplace_front(ptr);
        NodeMap_.emplace(ptr,NodeList_.begin());
    } else{     
        adjust(fd);
    }
}

void HeartConnect::adjust(int fd){
    assert(isSameThread(loop_->getThreadId()));
    if(fdMap_.find(fd) == fdMap_.end()){
        
    }else{
        // LOG_INFO("存在fd:%d",fd);
        auto ptr = fdMap_[fd];
        ptr->time_ = getNow();
        auto it = NodeMap_[ptr];
        NodeList_.erase(it);
        NodeList_.emplace_front(ptr);
        NodeMap_[ptr] = NodeList_.begin();
        // LOG_INFO("end");
    }
}

void HeartConnect::headRead(){
    assert(isSameThread(loop_->getThreadId()));
    uint64_t exp;
    ::read(timefd_, &exp, sizeof(uint64_t)); // 需要把给定时器的数据读出来

    struct timespec now;
    int ret = clock_gettime(CLOCK_REALTIME, &now);//获取时钟时间
    assert(ret != -1);
    LOG_INFO("timeout");
    
    while(!NodeList_.empty()){
        auto ptr = *NodeList_.rbegin();
        __time_t time = ptr->time_ + expire_;
        if(time < now.tv_sec){ // 未来的时间大才删除
            del(ptr);
        }
        else{
            break;
        }
    }
}

void HeartConnect::del(NodePtr ptr){
    assert(isSameThread(loop_->getThreadId()));
    LOG_INFO("timeout delete tcpClient");
    if(ptr->cPtr_.lock() != nullptr){
        auto tmp = ptr->cPtr_.lock();
        tmp->setState(CLIENT_STATUS::MUST_CLOSE_CONNECT);
        LOG_INFO("timeout call closeBack %d",tmp->getFd());
        ptr->callback_();
    } else{
        destroyConnect(ptr); // callback会自动调用了，提前离开的需要自己调用
    }
}

void HeartConnect::destroyConnect(NodePtr ptr){
    assert(isSameThread(loop_->getThreadId()));
    LOG_INFO("destroyConnect");
    if(NodeMap_.find(ptr) != NodeMap_.end()){
        auto it = NodeMap_[ptr];
        NodeList_.erase(it);
        fdMap_.erase(ptr->fd_);
        NodeMap_.erase(ptr);
    }
}

void HeartConnect::clientCloseCallback(int fd){
    assert(isSameThread(loop_->getThreadId()));
    if(fdMap_.find(fd) != fdMap_.end()){
        auto ptr = fdMap_[fd];
        destroyConnect(ptr);
    }
}
