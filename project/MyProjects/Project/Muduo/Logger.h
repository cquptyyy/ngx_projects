#ifndef MUDUO_BASE_LOGGER_H
#define MUDUO_BASE_LOGGER_H

#include "noncopyable.h"
#include <string>

//LOG_INFO("****%d,****%s",...);

#define LOG_INFO(logmsgFormat,...)\
do{\
  Logger& logger=Logger::getInstance();\
  logger.setLogLevel(INFO);\
  char buffer[1024];\
  snprintf(buffer,sizeof(buffer),logmsgFormat,##__VA_ARGS__);\
  logger.log(buffer);\
}while(0)

#define LOG_ERROR(logmsgFormat,...)\
do{\
  Logger& logger=Logger::getInstance();\
  logger.setLogLevel(ERROR);\
  char buffer[1024];\
  snprintf(buffer,sizeof(buffer),logmsgFormat,##__VA_ARGS__);\
  logger.log(buffer);\
}while(0)

#define LOG_FATAL(logmsgFormat,...)\
do{\
  Logger& logger=Logger::getInstance();\
  logger.setLogLevel(FATAL);\
  char buffer[1024];\
  snprintf(buffer,sizeof(buffer),logmsgFormat,##__VA_ARGS__);\
  logger.log(buffer);\
  exit(1);\
}while(0)

#ifdef MODE_DEBUG
  #define LOG_DEBUG(logmsgFormat,...)\
  do{\
    Logger& logger=Logger::getInstance();\
    logger.setLogLevel(DEBUG);\
    char buffer[1024];\
    snprintf(buffer,sizeof(buffer),logmsgFormat,## __VA_ARGS__);\
    logger.log(buffer);\
  }while(0)
#else
   #define LOG_DEBUG(logmsgFormat,...)
#endif

//日志等级
enum LogLevel{
  INFO,//正常信息
  ERROR,//错误信息
  FATAL,//致命信息
  DEBUG,//调试信息
};

//日志
class Logger:noncopyable{
public:
  //获取日志单例对象
  static Logger& getInstance();
  //设置日志等级
  void setLogLevel(int loglevel);
  //打印日志
  void log(std::string msg);
private:
  int logLevel_;
};

#endif