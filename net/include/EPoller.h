#pragma once
#include <vector>
#include <sys/epoll.h>
#include "Poller.h"
enum class State{
    ADD = 1,
    DEL = 1,
};

class EPoller : public Poller{
public:
    enum class State{
        NEW = 0,
        MOD,
        DEL,
    };
public:
    explicit EPoller();
    ~EPoller() = default;

    void poll(std::vector<Channel*> &artiveEvent) override;
    void updateEvent(int opt,Channel *channel) ;
    void updateChannel(Channel *channel) ;
private:
    std::vector<epoll_event> artiveEvent_;    
    int epollfd_;
    epoll_event events_;
};