#pragma once
#include <functional>

class EventLoop;

class Channel{
    using ReadCallback = std::function<void()>;
    using CloseCallback = std::function<void()>;
    using WriteCallback = std::function<void()>;
private:
    enum class EventStaue{
        NEW = 0,
        MOD,
        DEL,
    };
public:
    Channel(EventLoop* loop,int fd);
    void handleEvent();
    void setWriteCallback(WriteCallback cb){
        writeCallback_ = cb;
    }
    void setReadCallback(ReadCallback readCallback){
        readCallback_ = readCallback;
    }
    void setCloseCallback(CloseCallback cb) {
        closeCallback_ = cb;
    }
    void setCurEventState(int event){curEvents_ = event;}

    void disableReadEvent(){
        events_ &= ~0x001;
        update();
    }
    void disableWriteEvent(){
        events_ &= ~0x004;
        update();
    }
    void disableAll(){
        events_ = 0;
        update();
    }
    void enableReadEvent(){
        events_ |= 0x001;
        update();
    }
    void enableWriteEvent(){
        events_ |= 0x004;
        update();
    }
    void update();
    void setIndex (int index){index_ = index;}
    int getIndex()const{return index_;}
    int getFd()const{return fd_;}
    int getEvents()const{return events_;}
private:
    ReadCallback readCallback_;
    CloseCallback closeCallback_;
    WriteCallback writeCallback_;
    int index_ = (int) EventStaue::NEW;
    int events_ = 0;
    int curEvents_ = 0;
    EventLoop *loop_ =nullptr;
    int fd_ = 0;
};