#include <battery.h>
#include <llog.h>

uint64_t last_battery_read_time = 0;
uint8_t last_battery_read_percent = -1; // overflow, skill issue

// https://digitalconcepts.net.au/arduino/index.php?op=Battery
void Battery::get_battery_voltage() {
    float temp = 0;
    uint16_t volt;
    uint8_t pin;
    pin = ADC;

#if defined(CubeCell_Board_V2)
    pin = ADC_VBAT;
#endif

#if defined(CubeCell_Board)||defined(CubeCell_Capsule)||defined(CubeCell_BoardPlus)||defined(CubeCell_BoardPRO)||defined(CubeCell_GPS)||defined(CubeCell_HalfAA)||defined(CubeCell_Board_V2)
    /*
    * have external 10K VDD pullup resistor
    * connected to VBAT_ADC_CTL pin
    */

    pinMode(VBAT_ADC_CTL, OUTPUT);
    digitalWrite(VBAT_ADC_CTL, LOW);
#endif
    for (int i = 0; i < BATTERY_SAMPLES; i++) //read X times and get average
        temp += analogReadmV(pin);
    volt = temp / BATTERY_SAMPLES;

#if defined(CubeCell_Board)||defined(CubeCell_Capsule)||defined(CubeCell_BoardPlus)||defined(CubeCell_BoardPRO)||defined(CubeCell_GPS)||defined(CubeCell_HalfAA)||defined(CubeCell_Board_V2)
    pinMode(VBAT_ADC_CTL, INPUT);
#endif

    this->voltage = volt * 2;


    this->percentage = map(this->voltage, BATTERY_MIN, BATTERY_MAX, 0, 100);
}

void Battery::battery_read_task()
{
    if (millis() < (last_battery_read_time + BATTERY_READ_DELAY))
        return;
    
    get_battery_voltage();

    if (this->percentage != last_battery_read_percent)
        LLOG_DEBUG("Battery voltage: %u  %u%%", this->voltage, this->percentage);
    
    last_battery_read_percent = this->percentage;

    last_battery_read_time = millis();
}