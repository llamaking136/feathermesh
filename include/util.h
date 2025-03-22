#pragma once

#include <board-config.h>
#include <llog.h>
#include <network.h>

extern void setLED(uint8_t, uint8_t, uint8_t);

uint32_t generate_random_number(uint32_t max);
void printnhex(uint8_t *, size_t, uint32_t);