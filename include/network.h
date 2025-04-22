#pragma once

#include <stdint.h>
#include <RadioLib.h>

#include <meshtastic/mesh.pb.h>

#define MAX_PAYLOAD_SIZE 255
#define SLOT_TIME 50

struct MeshNetwork_PacketHeader
{
    uint32_t destination;
    uint32_t sender;
    uint32_t id;
    uint8_t flags;
    uint8_t hash;
    uint16_t reserved;
} __attribute__((packed));

struct MeshNetwork_PacketFlags
{
    uint8_t hops;
    bool want_ack;
    bool via_mqtt;
    uint8_t init_hops;
};

extern bool transmitting;
extern uint8_t* transmit_buffer;
extern SX1262 radio;
extern int transmission_state;
extern bool is_scanning;

uint32_t encode_position_precision(uint32_t);
uint32_t decode_position_precision(uint32_t);
uint16_t calculate_packet_delay(float);
uint8_t encode_flags(MeshNetwork_PacketFlags*);
void decode_flags(uint8_t, MeshNetwork_PacketFlags*);
bool manage_decoded_packet(int32_t, float, MeshNetwork_PacketHeader*, MeshNetwork_PacketFlags*, meshtastic_Data*);
void retransmit_packet(float, MeshNetwork_PacketHeader*, MeshNetwork_PacketFlags*, meshtastic_Data*);