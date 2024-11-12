#pragma once

class Socket
{
public:
    Socket(int sockfd_);
    bool setKeepAlive(bool on);
    bool setReuseAddr(bool on);
    bool setReusePort(bool on);
    int getFd() const { return sockfd_; }
    int accept();
    ~Socket();
private:
    int sockfd_;
};