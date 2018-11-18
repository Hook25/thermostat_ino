#include "clock.h"

Clock::Clock(){
    this->_info = (clock_info){ 0, 0, 0 };
}
Clock::Clock(tinyint day, tinyint hour, tinyint minute){
    this->_info = (clock_info){day, hour, minute};
}
clock_info Clock::GetInfo(){
    return this->_info;
}
void Clock::_check_alarms(){
    for(int i = 0; i<this->_alarm_count; i++){
        if(this->_info == this->_alarms[i].scheduled_time){
            this->_alarms[i].callback(this->_alarms[i].args);
        }
    }
}
void Clock::Schedule(func_ptr callback, void* args, tinyint d, tinyint h, tinyint m){
  clock_info tmp_cinfo = (clock_info){d, h, m};
  alarm_info a = (alarm_info){ callback, tmp_cinfo, args};
  if(this->_alarm_count < MAX_ALARMS){
    this->_alarms[this->_alarm_count] = a;
    this->_alarm_count ++;
  }
}
void Clock::MinuteTick(){
    int minute = (this->_info.minute+1);
    int of = minute / 60;
    minute %= 60;
    this->_info.minute = minute;
    int hour = (this->_info.hour + of);
    of = hour / 24;
    hour %= 24;
    this->_info.hour = hour;
    this->_info.day = (this->_info.day+ of)%7;
    this->_check_alarms();
}
