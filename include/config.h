#pragma once

#define SHOW_COMMAND_OUTPUT true
#define COMMAND_OUTPUT_COLOR true
#define REDACT_POSITIONS false

#define TX_ENABLED true

#define LED_PIN RGB
#define LED_ON 255
#define LED_OFF 0

#define FREQUENCY 906.875

#define BANDWIDTH 250.0
#define CODING_RATE 8
#define SPREADING_FACTOR 11

#define SYNC_WORD 0x2B
#define TRANSMIT_POWER_DBM 22

#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_CS P4_3
#define LORA_RST P5_7
#define LORA_DIO0 P4_6

#include <node_info.h>