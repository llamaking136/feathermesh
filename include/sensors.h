#pragma once

#include <Adafruit_BMP280.h>

#define SENSORS_MIN_READ_DELAY 5000
#define SENSORS_READ_DELAY 15 * 60 * 1000

extern Adafruit_BMP280 barometer;
extern bool barometer_present;