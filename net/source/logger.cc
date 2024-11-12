#include "logger.h"
#include "TimeStamp.h"


#include <iostream>

void logger::setLevel(int level){
    logLevel_  = level;
}


logger& logger::instance(){
    static logger logger;
    return logger;
}


void logger::print(std::string msg){
    switch (logLevel_)
    {
    case (int)level::INFO:{
        std::cout<<"[INFO]";
        break;
    }
    case (int)level::ERROR:{
        std::cout<<"[ERROR]";
        break;
    }
    case (int)level::FATAL:{
        std::cout<<"[FATAL]";
        break;
    }
    default:
        break;
    }
    std::cout<<" "<< TimeStamp::now() <<" " << msg <<std::endl;
}