#pragma once
#include<string>
#include<thread>
#include<sstream>


enum class level{
    INFO,
    ERROR,
    FATAL,
};

#define LOG_INFO(logmsgFormat,...)\
    do\
    {\
        logger& logger = logger::instance();\
        logger.setLevel((int)level::INFO);\
        char buf[1024] = {0}; \
        std::ostringstream oss;\
        oss << std::this_thread::get_id();\
        int LOG_N = snprintf(buf,1024,"%s [<%s>%s:%d] ",oss.str().c_str(),__FILE__,__func__,__LINE__);\
        snprintf(buf + LOG_N,1024 - LOG_N,logmsgFormat,##__VA_ARGS__); \
        logger.print(buf); \
    }while(0)


#define LOG_ERROR(logmsgFormat,...) \
    do \
    { \
        logger& logger = logger::instance(); \
        logger.setLevel((int)level::ERROR); \
        char buf[1024] = { 0 }; \
        snprintf(buf,sizeof(buf),logmsgFormat,##__VA_ARGS__); \
        logger.print(buf); \
    }while(0)


#define LOG_FATAL(logmsgFormat,...) \
    do \
    { \
        logger& logger = logger::instance(); \
        logger.setLevel((int)level::FATAL); \
        char buf[1024] = { 0 }; \
        snprintf(buf,sizeof(buf),logmsgFormat,##__VA_ARGS__); \
        logger.print(buf); \
        exit(-1);\
    }while(0)


class logger{
public:
    static logger& instance();
    void print(std::string msg);
    void setLevel(int level);
private:
    int logLevel_;

    
};