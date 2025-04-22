#pragma once

#include <Arduino.h>
#include <config.h>
#include <pb_helper.h>
#include <llog.h>
#include <channels.h>
#include <CubeCell_NeoPixel.h>
#include <RadioLib.h>
#include <board-config.h>
#include <util.h>
#include <crypt.h>
#include <sensors.h>
#include <transmit_queue.h>

#include <meshtastic/mesh.pb.h>

extern CubeCell_NeoPixel led;

extern SX1262 radio;

extern uint32_t transmit_queue_last_tx_time;
extern uint32_t transmit_queue_delay;

extern int transmission_state;
extern bool transmitting;
extern bool is_receiving;

extern int receive_state;
extern bool did_receive;
extern bool did_fail_to_decode;

extern bool operation_flag;
extern bool is_scanning;

extern uint32_t transmit_node_id_delay;
extern uint64_t transmit_node_id_time;

extern uint64_t led_start_time;
extern uint32_t did_receive_time;
extern uint16_t led_receive_delay;

extern uint64_t print_timer;
extern uint32_t print_delay;

extern uint32_t last_irq_value;

void transmit_nodeinfo_task();
void transmit_position_task();
void transmitter_task();
void receive_task();
void led_blink_task();
uint8_t led_amplitude(uint8_t, uint64_t);