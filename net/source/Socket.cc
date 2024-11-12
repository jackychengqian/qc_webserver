#include "logger.h"
#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>      
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;

Socket::Socket(int sockfd_):sockfd_(sockfd_){}

bool Socket::setKeepAlive(bool on){
    int optval = on ? 1 : 0;
    int ret = setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,static_cast<socklen_t>(sizeof(optval)));
    if(ret < 0){
        LOG_ERROR("setKeepAlive error");
        return false;
    }
    return true;
}

bool Socket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    int ret = setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval,static_cast<socklen_t>(sizeof(optval)));
    if(ret < 0){
        LOG_ERROR("setReuseAddr error");
        return false;
    }
    return true;
}

bool Socket::setReusePort(bool on){
    int optval = on ? 1 : 0;
    int ret = setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&optval,static_cast<socklen_t>(sizeof(optval)));
    if(ret < 0){
        LOG_ERROR("setReusePort error");
        return false;
    }
    return true;
}

int Socket::accept(){
    int connfd = ::accept4(sockfd_,nullptr,nullptr,SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0){
        LOG_ERROR("accept error");
        return -1;
    }
    return connfd;
}

Socket::~Socket(){
    ::close(sockfd_);
}