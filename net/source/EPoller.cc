#include "EPoller.h"
#include "Channel.h"
#include "logger.h"

#include <functional>
#include <signal.h>

EPoller::EPoller():artiveEvent_(16),epollfd_(::epoll_create(8))
{}

void EPoller::poll(std::vector<Channel*> &artiveEvent){
    size_t num = epoll_wait(epollfd_,&(*artiveEvent_.begin()),artiveEvent_.size(),-1);
    if (num == UINT64_MAX)
    {
        LOG_ERROR("epoll_wait error %d",errno);
        return;
    }
    
    else if(num == 0){
        LOG_INFO("epoll_wait timeout %s",__func__);
    }

    if(num >= artiveEvent_.size()){
        artiveEvent_.resize(artiveEvent_.size()*2);
    }

    for(size_t i = 0; i < num; ++i){
        Channel *channel = static_cast<Channel*>(artiveEvent_[i].data.ptr);
        channel->setCurEventState(artiveEvent_[i].events);
        artiveEvent.push_back(channel);
    }
}


void EPoller::updateEvent(int opt,Channel *channel){
    epoll_event event;
    event.data.fd = channel->getFd();
    event.data.ptr = channel;
    event.events = channel->getEvents();
    ::epoll_ctl(epollfd_,opt,channel->getFd(),&event);
}

void EPoller::updateChannel(Channel* channel){
    int index = channel->getIndex();
    if(index == (int)State::NEW){
        channel->setIndex((int)State::MOD);
        updateEvent(EPOLL_CTL_ADD,channel);
        channels_.emplace(channel->getFd(),channel);
    }
    else if(index == (int)State::DEL){
        updateEvent(EPOLL_CTL_DEL,channel);
        channels_.erase(channel->getFd());
    }
    else{
        updateEvent(EPOLL_CTL_MOD,channel);
    }
}