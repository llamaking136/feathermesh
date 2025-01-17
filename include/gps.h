#pragma once

#include <Arduino.h>
#include <TinyGPS++.h>
#include <softSerial.h>
#include <stdint.h>
#include <llog.h>

#define GPS_TASK_TX_DELAY 15 * 60 * 1000

const uint8_t GPS_rx = GPIO0;
const uint8_t GPS_tx = GPIO5;

extern TinyGPSPlus GPS;
extern softSerial GPSSerial;

void init_gps();
void read_gps_buffer();
void print_gps_task(bool);
// stolen from chatcbt
time_t convertToUnixTime(int year, int month, int day, int hour, int minute, int second);