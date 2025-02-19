#include "Logger.h"
#include <iostream>

//获取单例日志对象
Logger& Logger::getInstance(){
  static Logger logger;
  return logger;
}

//设置日志等级
void Logger::setLogLevel(int logLevel){
  logLevel_=logLevel;
}

//打印日志信息
void Logger::log(std::string msg){
  switch(logLevel_){
  case INFO:
  std::cout<<"[INFO]";
  break;
  case ERROR:
  std::cout<<"[ERROR]";
  break;
  case FATAL:
  std::cout<<"[FATAL]";
  break;
  case DEBUG:
  std::cout<<"[DEBUG]";
  break;
  default:
  break;
  }
  std::cout<<"print time"<<":"<<msg<<std::endl;
}
