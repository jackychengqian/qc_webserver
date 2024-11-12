#include "EventLoop.h"
#include "logger.h"
#include "EPoller.h"
#include "Channel.h"
#include "TimerQueue.h" 

#include <errno.h>
#include <sys/eventfd.h>
#include <unistd.h>
using namespace std;

static int createEventfd(){
    int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        LOG_FATAL("eventfd error:%d \n",errno);
    }
    return evtfd;
}

static  bool isSameThread(const std::thread::id &tid){
    return tid == std::this_thread::get_id();
}

EventLoop::EventLoop():epoller_(std::make_unique<EPoller>()),
                        wakefd_(createEventfd()),
                        threadId_(std::this_thread::get_id()),
                        wakeupChannel_(std::make_unique<Channel>(this,wakefd_)),
                        timerQueue_(std::make_unique<TimerQueue>(this))
{

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReadEvent();
}

void EventLoop::loop(){
    while(true){
        std::vector<Channel*> activeChannel;
        epoller_->poll(activeChannel);
        for(auto &channel:activeChannel){
            channel->handleEvent();
        }
        doPendingFunctor();
    }
}

void EventLoop::updateChannel(Channel *channel){
    epoller_->updateChannel(channel);
}

void EventLoop::runInLoop(const Function &cb){
    if(isSameThread(threadId_)){
        cb();
    }else{
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Function &cb){
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
    if(!isSameThread(threadId_)){
        wakeup();
    }
}

void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = ::write(wakefd_,&one,sizeof(one));
    if(n != sizeof(one)){
        LOG_ERROR("EventLoop::wakeup() writes %ld bytes instead of 8",n);
    }
}

void EventLoop::handleRead(){
    uint64_t one = 1;
    size_t n = ::read(wakefd_,&one,sizeof(one));
    if(n != sizeof(one)){
        LOG_ERROR("handleRead not 8");
    }
}

void EventLoop::doPendingFunctor(){
    std::vector<Function> functors;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(auto &functor:functors){
        functor();
    }
}

void EventLoop::addTimer(timerCallback cb,int expire,bool repeat){
    timerQueue_->addTimer(cb,expire,repeat);
}


EventLoop::~EventLoop(){

}