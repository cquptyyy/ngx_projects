#include "TimeStamp.h"
#include <ctime>


TimeStamp::TimeStamp():microSecondsSinceEpoch_(0){}
TimeStamp::TimeStamp(int64_t microSecondsSinceEpoch):microSecondsSinceEpoch_(microSecondsSinceEpoch){}
TimeStamp TimeStamp::now(){
  return TimeStamp(time(nullptr));
}

std::string TimeStamp::toString(){
  char buffer[128]={0};
  struct tm* tm_time=localtime(&microSecondsSinceEpoch_);
  snprintf(buffer,sizeof(buffer),"%4d-%02d-%02d %2d:%02d:%02d",
  tm_time->tm_year+1900,
  tm_time->tm_mon+1,
  tm_time->tm_mday,
  tm_time->tm_hour,
  tm_time->tm_min,
  tm_time->tm_sec
  );
  return buffer;
}


