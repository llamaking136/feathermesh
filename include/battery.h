#pragma once

#include <Arduino.h>
#include <stdint.h>

#define BATTERY_SAMPLES 20
#define BATTERY_READ_DELAY 5 * 60 * 1000

#define BATTERY_MIN 3000
#define BATTERY_MAX 4200

class Battery
{
private:
    uint16_t voltage;
    uint8_t percentage;

public:
     Battery() { voltage = 0; };
    ~Battery() {};

    inline uint16_t get_voltage() { return this->voltage; }
    inline uint8_t get_percentage() { return this->percentage; }
    void get_battery_voltage();
    void battery_read_task();
};

extern Battery battery;