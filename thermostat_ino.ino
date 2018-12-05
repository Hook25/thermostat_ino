#define DEBUG 1

#if DEBUG
#define LOG(x) Serial.println(x)
#else
#define LOG(x) 
#endif

#include <sched.h>
#include "clock.h"
#include <DHT.h>

#define MAX_COMMAND_SIZE 20
#define THERMAL_PIN 2
#define DHTPIN 7
#define DHTTYPE DHT22
#define ARGS_DELIMITER ','
#define COMMAND_DELIMITER ";"

#define SET_TIME_ID 1

LittleScheduler ls;
Clock c(1, 12, 17);
DHT dht(DHTPIN, DHTTYPE);

float ambiental_temperature = 99;
float target_temperature=10;
float high_temperature = 90;
float low_temperature  = 10;
bool on = false;
bool locked = false;

void setup() {
  dht.begin();
  #if DEBUG
  Serial.begin(9600);
  #endif
  pinMode(THERMAL_PIN, OUTPUT);
  pinMode(DHTPIN, INPUT);
  
  ls.add_task(MinuteTickWrapper, 60, 0, NULL);   //ticks a minute once every 60 seconds
  ls.add_task(ReadTemperature, 1, 0, NULL);      //reads the temperature once every second
  ls.add_task(EnableDisableThermal, 1, 0, NULL); //sets the on status according to the temperature
  ls.add_task(apply_status, 120, 0, NULL);        //turns on or off the relay according to the on and locked variable
}

void apply_status(void*){
  if(!locked){
    digitalWrite(THERMAL_PIN, on ? HIGH : LOW);
  }
}

void make_high(void*){
  target_temperature = high_temperature;
}
void make_low(void*){
  target_temperature = low_temperature;
}
void MinuteTickWrapper(void*){
  c.MinuteTick();
  auto info = c.GetInfo();
  LOG("Day: " + String(info.day) + " Hour: " + String(info.hour) + 
  " Minute: " + String(info.minute) + " Temperature " + String(target_temperature) +
  " Ambiental Temperature " + String(ambiental_temperature));
}
void EnableDisableThermal(void*){
  if(ambiental_temperature < target_temperature){
    on = true;
  }else if(ambiental_temperature > target_temperature + 2){
    on = false;
  }
}
void ReadTemperature(void*){
  ambiental_temperature = dht.readTemperature();
}

//realistically unoverflowable, overflows once every 130 years
unsigned long seconds(){
  static unsigned long old_time, old_tick, second;
  static bool inited;
  if(!inited){
    old_time = millis();
    inited = true;
  }
  //LOG(String(old_tick));
  unsigned long new_time = millis();
  unsigned long elapsed = new_time - old_tick; //safe cause of long overflow
  if((elapsed / 1000) > 0){
    second += elapsed / 1000;
    old_tick = new_time;
  }
  //LOG("seconds: " + String(second));
  old_time = new_time;
  return second;
}
int set_time(char* command){
  Serial.println(command);
  char* day = command;
  char* hour = strchr(command, ARGS_DELIMITER);
  if(!hour){ Serial.println(*hour); return 1; }
  *hour = 0;
  hour++;
  char* minute = strchr(hour, ARGS_DELIMITER);
  if(!minute){ Serial.println(*minute); return 1;}
  *minute = 0;
  minute++;
  int day_i = atoi(*day);
  int hour_i = atoi(*hour);
  int minute_i = atoi(*minute);
  Serial.println(*day);
  Serial.println(*hour);
  Serial.println(*minute);
  
  return 0;
}

//reads commands from serial, command format is
//COMMAND_ID,arg1, arg2, ..., argn;
void command_worker(){
  if(Serial.available()){
    char input[MAX_COMMAND_SIZE+1];
    byte size = Serial.readBytes(input, MAX_COMMAND_SIZE);
    input[size] = 0; //null terminate string
    char* element = strtok(input, COMMAND_DELIMITER);
    while(element != 0){
      int res = 1;
      char* command = strchr(element, ARGS_DELIMITER);
      if(command != 0){
        *command = 0; //delimit the command from the args
        int command_id = atoi(element);
        command ++; //point to the args
        switch(command_id){
          case SET_TIME_ID:
          res = set_time(command);
          break;
          default:
          Serial.println("Error parsing command");
          Serial.flush();
          break;
        }
        if(res == 1){
          Serial.println("Error in syntax");
        }
      }
      element = strtok(0, COMMAND_DELIMITER);
    }
  }
}

void loop() {
  ls.run_sched(seconds());
  command_worker();
}
