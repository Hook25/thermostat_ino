#ifndef CLOCK
#define CLOCK
#define MAX_ALARMS 20
typedef unsigned char tinyint;
typedef void (*func_ptr)(void*);
struct clock_info{
    tinyint day;
    tinyint hour;
    tinyint minute;
    bool operator==(const clock_info &other){
        return day == other.day && hour == other.hour && minute == other.minute;
    }
};

struct alarm_info{
    func_ptr callback;
    clock_info scheduled_time;
    void* args;
};

class Clock{
private:
    clock_info _info;
    alarm_info _alarms[MAX_ALARMS];
    unsigned int _alarm_count;
    void _check_alarms();
public:
    clock_info GetInfo();
    Clock();
    Clock(tinyint day, tinyint hour, tinyint minute);
    void SetTime(tinyint day, tinyint hour, tinyint minute);
    void MinuteTick();
    void Schedule(func_ptr, void*, tinyint, tinyint, tinyint);
};
#endif
