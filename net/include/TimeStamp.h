#pragma once
#include <string>
#include <chrono>
#include <sys/time.h>



class TimeStamp{
public:
    static std::string now(){
        char buff[128] = {0};
        time_t microSecondsSinceEpch = ::time(NULL);
        tm* tm_time = localtime(&microSecondsSinceEpch);
        snprintf(buff,sizeof(buff),"%4d/%02d/%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,
        tm_time -> tm_mon + 1,
        tm_time->tm_mday,
		tm_time->tm_hour,
		tm_time->tm_min,
		tm_time->tm_sec
		);

        return buff;
    }
    static std::string getUTC(){
        auto now = std::chrono::system_clock::now();

        std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
        tm* tm_time = localtime(&nowTime);
        char buff[128] = {0};
        strftime(buff,sizeof(buff),"%a,%d %b %Y %X UTC ",tm_time);
        return buff;
    }
    static std::uint64_t nowToSecond(){
        struct timeval tv;
        gettimeofday(&tv, NULL);
        std::uint64_t seconds = tv.tv_sec;
        return seconds;
    }
private:
};