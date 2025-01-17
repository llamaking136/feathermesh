#include <tasks.h>

void led_blink_task()
{
    if (led_start_time == 0)
    {
        led_start_time = millis();
    }

    if (transmitting)
        setLED(255, 0, 0);
    else if (did_receive)
    {
        setLED(did_fail_to_decode ? 255 : 0, 255, 0);
        if (did_receive_time == 0)
            did_receive_time = millis();
    }
    else
    {
        if (led_start_time == 0)
            led_start_time = millis();
        uint8_t amplitude = led_amplitude(20, millis() - led_start_time);

        setLED(amplitude, amplitude, amplitude);
    }

    if (millis() >= (did_receive_time + led_receive_delay))
    {
        did_receive = false;
        did_receive_time = 0;
        did_fail_to_decode = false;
    }
}

// input: time in milliseconds
// output: 0 to 255, representing amplitude of led
uint8_t led_amplitude(uint8_t max_value, uint64_t msec)
{
    float amplitude = max_value * sin((1.0 / 2000.0) * PI * msec);
    return amplitude < 0 ? 0 : (uint8_t)amplitude;
}