#define DEBUG 1

#if DEBUG
#define LOG(x) Serial.println(x)
#else
#define LOG(x) 
#endif

#include <sched.h>
#include "clock.h"
#include <DHT.h>

#define THERMAL_PIN 2
#define DHTPIN 7
#define DHTTYPE DHT22

LittleScheduler ls;
Clock c(1, 12, 17);
DHT dht(DHTPIN, DHTTYPE);

float ambiental_temperature = 99;
float target_temperature=10;
float high_temperature = 90;
float low_temperature  = 10;

void setup() {
  dht.begin();
  #if DEBUG
  Serial.begin(9600);
  #endif
  pinMode(THERMAL_PIN, OUTPUT);
  pinMode(DHTPIN, INPUT);
  
  ls.add_task(MinuteTickWrapper, 60, 0, NULL);
  ls.add_task(ReadTemperature, 1, 0, NULL);
  ls.add_task(EnableDisableThermal, 10, 0, NULL);
  
}
void make_high(void*){
  target_temperature = high_temperature;
}
void make_low(void*){
  target_temperature = low_temperature;
}
void turn_on(){
  digitalWrite(THERMAL_PIN, HIGH);
}
void turn_off(){
  digitalWrite(THERMAL_PIN, LOW);
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
    turn_on();
  }else if(ambiental_temperature > target_temperature -2){
    turn_off();
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
void loop() {
  ls.run_sched(seconds());
  
}
