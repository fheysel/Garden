/*
 * Autonomous Garden Project
 * By Fiona Heysel
 * December 23 2018
 * Version 1
 * 
 * This version includes an rtc to provide accurate timekeeping and interrupts.
 * On alarm interrupt the system will toggle the light and water the plant.
 */

#include <Servo.h>
#include <Wire.h>
#include "ds3231.h"

#define BUFF_MAX 256 //size of rtc buffer
#define LIGHT_PIN 7
#define VALVE_PIN 3

#define VALVE_OPEN 95//TESTED VALUES
#define VALVE_CLOSED 180

Servo valve;

// time when to wake up
uint8_t wake_HOUR = 7;
uint8_t wake_MINUTE = 0;
uint8_t wake_SECOND = 0;

// how often to refresh the info on stdout (ms)
unsigned long prev = 5000, interval = 60000;


void setup() {
  Serial.begin(9600);
  Wire.begin();

  //RTC SET UP
  DS3231_init(DS3231_CONTROL_INTCN);
  DS3231_clear_a1f();
  DS3231_clear_a2f();
  set_alarm();
  set_time();//ONLY RUN THIS ON INITIAL SETUP! LEAVE THIS COMMENTED!!!

  //SERVO SET UP
  valve.attach(VALVE_PIN);
  valve.write(VALVE_CLOSED);

  //LED SET UP
  pinMode(LIGHT_PIN, OUTPUT);
  struct ts t;
  DS3231_get(&t);
  if(t.hour >= 19 || t.hour <= 7){
    digitalWrite(LIGHT_PIN, HIGH);// if current time is between 7 pm and 7 am, start light on
  }
  else{
    digitalWrite(LIGHT_PIN, LOW);      
  }
}

void loop() {
  unsigned long now = millis();

  // every minute display data and check if an interrupt has occured
  if ((now - prev > interval) && (Serial.available() <= 0)) {
      //printTime();

      if (DS3231_triggered_a1()) {
        DS3231_clear_a1f(); // clear source of interrupt
        digitalWrite(LIGHT_PIN, 0);
        water();
      }

      if (DS3231_triggered_a2()) {
        DS3231_clear_a2f();//clear source of interrupt
        digitalWrite(LIGHT_PIN, 1);
      }
      prev = now;
  }
}

void set_alarm(){
  // flags define what calendar component to be checked against the current time in order
  // to trigger the alarm - see datasheet
  // A1M1 (seconds) (0 to enable, 1 to disable)
  // A1M2 (minutes) (0 to enable, 1 to disable)
  // A1M3 (hour)    (0 to enable, 1 to disable) 
  // A1M4 (day)     (0 to enable, 1 to disable)
  // DY/DT          (dayofweek == 1/dayofmonth == 0)
  uint8_t flags1[5] = { 0, 0, 0, 1, 1 }; //Indicates to only check when the hours match
  uint8_t flags2[4] = { 0, 0, 1, 1 }; //Alarm2 does not have a seconds value, it starts with minutes, therefore only matches hours as well

  DS3231_set_a1(wake_SECOND, wake_MINUTE, wake_HOUR, 0, flags1);//set Alarm1
  DS3231_set_a2(wake_MINUTE, wake_HOUR + 12, 0, flags2);//set Alarm2 wake_HOUR + 12 so it wakes at 7pm
    
  DS3231_set_creg(DS3231_CONTROL_INTCN | DS3231_CONTROL_A1IE | DS3231_CONTROL_A2IE);//Activates both alarms
}

void set_time(){
  struct ts t;
  t.sec = 0;
  t.min = 45;
  t.hour = 0b;
  t.wday = 7;//sunday, user defined.
  t.
  
}

void water(){
  valve.write(VALVE_OPEN);
  delay(5000);
  valve.write(VALVE_CLOSED);
}

void printTime(){
  char buff[BUFF_MAX];
  struct ts t; //struct ts defined in ds3231.h
  DS3231_get(&t);
  // display current time
  snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
  Serial.println(buff);   

  // display a1 debug info
  DS3231_get_a1(&buff[0], 59);
  Serial.println(buff);
}
