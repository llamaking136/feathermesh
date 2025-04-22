#include <Arduino.h>
#include <RadioLib.h>
#include <CubeCell_NeoPixel.h>
#include <llog.h>
#include <board-config.h>
#include <network.h>
#include <util.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <pb_helper.h>
#include <transmit_queue.h>
#include <config.h>
#include <channels.h>
#include <crypt.h>
#include <commands.h>
#include <tasks.h>
#include <gps.h>
#include <battery.h>
#include <sensors.h>
#include <airtime.h>


CubeCell_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

SX1262 radio = new Module(RADIO_NSS, RADIO_DIO_1, RADIO_RESET, RADIO_BUSY);

TransmitQueue transmit_queue;
uint32_t transmit_queue_last_tx_time = 0;
uint32_t transmit_queue_delay = 500;

int transmission_state = RADIOLIB_ERR_NONE;
bool transmitting = false;
bool is_receiving = false;

int receive_state = RADIOLIB_ERR_NONE;
bool did_receive = false;
bool did_fail_to_decode = false;

bool operation_flag = false;
bool is_scanning = false;


uint32_t transmit_node_id_delay = 60 * 60 * 1000;
uint64_t transmit_node_id_time = 0;


uint64_t led_start_time = 0;
uint32_t did_receive_time = 0;
uint16_t led_receive_delay = 100;

uint64_t print_timer = 0;
uint32_t print_delay = 500;

uint32_t last_irq_value = 0;

Battery battery;
Airtime airtime;
Adafruit_BMP280 barometer;
bool barometer_present = false;

void set_operation_flag()
{
    if (is_scanning && !transmitting)
        return;
    
    operation_flag = true;
}

void setLED(uint8_t red, uint8_t green, uint8_t blue)
{
    led.setPixelColor(0, led.Color(red, green, blue));
    led.show();
}

void init_success()
{
	setLED(LED_ON, LED_OFF, LED_OFF); // red
	delay(125);
	setLED(LED_OFF, LED_ON, LED_OFF); // green
	delay(125);
	setLED(LED_OFF, LED_OFF, LED_ON); // blue
	delay(125);
	setLED(LED_ON, LED_OFF, LED_ON); // purple
	delay(125);
	setLED(LED_OFF, LED_OFF, LED_OFF); // off
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    init_show_copyright();

    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    led.begin();
    init_success();

    // Initialize the LoRa module
    int status = radio.begin();
    if (status != RADIOLIB_ERR_NONE)
    {
        LLOG_CRITICAL("LoRa failed to begin, error code: %d", status);
        for (;;);
    }

    radio.setFrequency(FREQUENCY);
    radio.setBandwidth(BANDWIDTH);
    radio.setCodingRate(CODING_RATE);
    radio.setSpreadingFactor(SPREADING_FACTOR);
    radio.setSyncWord(SYNC_WORD);
    radio.setOutputPower(TRANSMIT_POWER_DBM);
    radio.setRxBoostedGainMode(true);

    radio.setDio1Action(set_operation_flag);

    LLOG_INFO("LoRa initialized.");

    init_gps();

    bool barometer_status = barometer.begin(0x76);
    if (barometer_status == false)
    {
        LLOG_WARNING("No barometer found on I2C bus.");
    }
    else
        barometer_present = true;

    init_channels();
#include <channel_keys.txt>

    receive_state = radio.startReceive();
    if (receive_state == RADIOLIB_ERR_NONE)
    {
        LLOG_INFO("Starting to receive.");
    }
    else
    {
        LLOG_ERROR("Error receiving, error code: %d", receive_state);
        for (;;);
    }
}

void loop()
{
    // if (millis() >= (print_timer + print_delay))
    // {
    //     LLOG_DEBUG("irq_flags: %u", radio.getIrqFlags());
    //     LLOG_DEBUG("transmitting: %u  is_scanning: %u  operation_flag: %u", transmitting, is_scanning, operation_flag);

    //     print_timer = millis();
    // }

    if (last_irq_value != radio.getIrqFlags())
    {
        // LLOG_DEBUG("irq_flags: %u", radio.getIrqFlags());
        last_irq_value = radio.getIrqFlags();

        // dont know why the node does this but sometimes it gets stuck thinking its
        // transmitting. maybe a fix?
        if ((last_irq_value & RADIOLIB_SX126X_IRQ_TX_DONE) == RADIOLIB_SX126X_IRQ_TX_DONE && transmitting)
        {
            // LLOG_WARNING("Got into transmitting stuck state! Trying to bail.");
            operation_flag = true;
            radio.clearIrqFlags(0);
        }

        // again, another fix.
        // i dont know why but sometimes it gets stuck receiving like transmitting.
        // hopefully a fix, it only happens once in a lifetime
        if ((last_irq_value & RADIOLIB_SX126X_IRQ_RX_DONE) == RADIOLIB_SX126X_IRQ_RX_DONE &&
            (last_irq_value & RADIOLIB_SX126X_IRQ_HEADER_VALID) == RADIOLIB_SX126X_IRQ_HEADER_VALID &&
            !transmitting)
        {
            // these happen so often, no need to hold up the log
            // fix seems to work!
            // LLOG_WARNING("Got into receiving stuck state! Trying to bail.");
            operation_flag = true;
            radio.clearIrqFlags(0);
        }
    }

    read_serial_buffer();

    // read_gps_buffer();
    // print_gps_task(false);

    led_blink_task();

    battery.battery_read_task();

    transmit_nodeinfo_task();

    // no gps for now
    // transmit_position_task();

    transmitter_task();

    receive_task();

    airtime.calculate_airtime_task();

    operation_flag = false;
}