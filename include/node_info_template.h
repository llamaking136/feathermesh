#pragma once

#include <stdint.h>

// a number between 0x00 and 0x5c, signifies the hardware model.
// check HardwareModel in mesh.proto
const uint8_t device_hardware_id = 0x00;

// strings representing a short and long name for the current node.
// long_name can have 39 characters and short_name can have 4 characters.
const char* const device_long_name = "Meshtastic ffff";
const char* const device_short_name = "ffff";

// a near-random string of 8 bytes, signifies an ID of the current node .
const uint32_t device_id = 0xFFFFFFFF;

// a copy of device_id but in string form with a bang character at the front.
const char* const device_str_id = "!ffffffff";

// frankly i have no idea what this is used for, meshtastic get your game on
// an array of 6 bytes representing a thingamabob.
const uint8_t device_mac_address[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// a number between 1 and 7, it signifies how many hops a packet from this node
// can reach in the mesh.
const uint8_t init_hops = 7;