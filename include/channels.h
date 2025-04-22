#pragma once

#include <stdint.h>

#define MAX_CHANNELS 10
#define MAX_CHANNEL_NAME 16
#define MAX_KEY_SIZE 32

struct Channel
{
    uint8_t* name;
    uint8_t name_length;

    uint8_t* key;
    uint8_t key_length;

    uint8_t hash;
};

extern Channel channels[];
void add_channel(uint8_t*, uint8_t, uint8_t*, uint8_t);
void add_channel_b64(const char*, const char*);
void init_channels();
uint8_t calculate_channel_hash(uint8_t*, uint8_t, uint8_t*, uint8_t);
Channel* find_channel(uint8_t);