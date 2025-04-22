#pragma once

#include <stdint.h>
#include <meshtastic/mesh.pb.h>

#define MAX_NODE_NUM 63
#define NODEDB_STRUCT_VERSION 1

struct Node
{
    uint32_t id;
    uint8_t long_name[40];
    uint8_t short_name[5];

    // 0 is direct
    uint8_t hops_away;
    // seconds since last packet
    uint64_t last_seen;

    meshtastic_HardwareModel model;
    meshtastic_Config_DeviceConfig_Role role;

    uint8_t public_key[32];
} __attribute__((packed));

class _NodeDB
{
private:
    static const uint32_t pool_size = MAX_NODE_NUM;
    Node pool[pool_size];
    
    uint32_t total_nodes = 0;

    Node* find_node(uint32_t);

public:
     _NodeDB() { memset(pool, 0, sizeof(Node) * pool_size); }
    ~_NodeDB() {}

    bool add_node(Node*, bool);
    bool remove_node(uint32_t);

    bool node_exists(uint32_t);
    Node* get_node(uint32_t);

    const uint32_t get_total_nodes() { return this->total_nodes; }
    const uint32_t get_pool_size() { return this->pool_size; }
};

extern _NodeDB NodeDB;