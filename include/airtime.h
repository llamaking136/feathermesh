#pragma once

#include <stdint.h>
#include <network.h>

#define AIRTIME_SAMPLE_DELAY 10000

class Airtime
{
private:
    uint32_t rx_time;
    uint32_t tx_time;
    uint16_t rx_bytes;
    uint16_t tx_bytes;
    uint64_t last_update = 0;
    uint8_t airtime_percent = 0;
    uint8_t last_airtime_percent = 255;

public:
     Airtime() {};
    ~Airtime() {};

    void calculate_airtime_task();
    inline void add_rx_bytes(uint16_t bytes) { this->rx_bytes += bytes; this->rx_time += (radio.getTimeOnAir(bytes) / 1000); }
    inline void add_tx_bytes(uint16_t bytes) { this->tx_bytes += bytes; this->tx_time += (radio.getTimeOnAir(bytes) / 1000); }
    inline uint8_t get_airtime() { return this->airtime_percent; }
};

extern Airtime airtime;