#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <network.h>
#include <util.h>

#define TRANSMIT_QUEUE_LEN 10

struct transmit_queue_record
{
    uint16_t payload_length;
    uint8_t* payload;
    MeshNetwork_PacketHeader header;
    uint64_t time_to_transmit;
};

class TransmitQueue
{
private:
    transmit_queue_record* queue_ptr;
    uint8_t queue_length;
    bool move_indexes(uint8_t, uint8_t);

public:
     TransmitQueue();
    ~TransmitQueue() { free(this->queue_ptr); }

    inline uint8_t size() { return queue_length; }
    inline bool is_empty() { return queue_length == 0; }
    inline void clean() { memset(this->queue_ptr, 0, sizeof(transmit_queue_record) * TRANSMIT_QUEUE_LEN); }

    bool push(transmit_queue_record*);
    bool pop(transmit_queue_record*);
};

extern TransmitQueue transmit_queue;