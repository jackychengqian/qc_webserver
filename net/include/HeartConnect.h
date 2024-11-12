#pragma once
#include <list>
#include <unordered_map>
#include <functional>
#include <memory>

#include "nocopyable.h"
#include "logger.h"
class Channel;
class EventLoop;
class TcpClient;

struct Node
{
    __time_t time_ = 0;
    int fd_ = 0;
    std::weak_ptr<TcpClient> cPtr_;
    std::function<void()> callback_;
    bool operator<(const std::shared_ptr<Node>& lhs)const
    {return this->time_ < lhs->time_;}
    ~Node(){
        LOG_INFO("head detection delete");
    }
};

class HeartConnect : public noncopyable{
    using NodePtr = std::shared_ptr<Node>;
public:
    HeartConnect(EventLoop* loop,int expire = 200);
    ~HeartConnect() = default;
    void add(int fd,std::shared_ptr<TcpClient> cPtr,std::function<void()> callback);

    void headRead();

    int getTimeFd()const {return timefd_;}

    NodePtr getNodePtr(int fd){return fdMap_[fd];}

    void clientCloseCallback(int fd);
private:
    void del(NodePtr ptr);
    void destroyConnect(NodePtr ptr);
    void adjust(int fd);
private:
    std::list<NodePtr> NodeList_;
    std::unordered_map<NodePtr,std::list<NodePtr>::iterator> NodeMap_;
    std::unordered_map<int,NodePtr> fdMap_;
    int timefd_;

    int expire_;
    EventLoop* loop_;
    std::unique_ptr<Channel> channel_;
};