#ifndef MUDUO_BASE_TIMESTAMP_H
#define MUDUO_BASE_TIMESTAMP_H
#include <iostream>
#include <string>
#include <cstdint>

class TimeStamp{
public:
  TimeStamp();
  TimeStamp(int64_t microSecondsSinceEpoch);
  static TimeStamp now();
  std::string toString();
private:
  int64_t microSecondsSinceEpoch_;
};
#endif