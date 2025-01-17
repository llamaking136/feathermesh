#include <airtime.h>
#include <Arduino.h>
#include <llog.h>

void Airtime::calculate_airtime_task()
{
    if (this->last_update == 0)
        this->last_update = millis() + AIRTIME_SAMPLE_DELAY;
    
    if (millis() < (this->last_update + AIRTIME_SAMPLE_DELAY))
        return;

    this->airtime_percent = (uint8_t)(((float)(this->rx_time + this->tx_time) / (float)AIRTIME_SAMPLE_DELAY) * 100.0f);
    this->last_update = millis();

    if (this->airtime_percent != this->last_airtime_percent)
        LLOG_DEBUG("Airtime: %u%%", this->airtime_percent);

    this->rx_time = 0;
    this->tx_time = 0;
    this->rx_bytes = 0;
    this->tx_bytes = 0;
    this->last_airtime_percent = this->airtime_percent;
}