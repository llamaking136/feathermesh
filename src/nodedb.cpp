#include <nodedb.h>
#include <llog.h>

_NodeDB NodeDB;

Node* _NodeDB::find_node(uint32_t id)
{
    for (size_t i = 0; i < this->pool_size; i++)
    {
        if (this->pool[i].id == id)
            return &this->pool[i];
    }
    return nullptr;
}

bool _NodeDB::add_node(Node* node, bool overwrite = false)
{
    Node* possible_node = this->find_node(node->id);
    if (possible_node != nullptr && !overwrite)
    {
        LLOG_ERROR("Trying to overwrite existing node !%08x (%s)! Ignoring request.", node->id, node->short_name);
        return false;
    }
    else if (possible_node != nullptr && overwrite)
    {
        LLOG_WARNING("Overwriting existing node !%08x (%s).", node->id, node->short_name);
    }

    Node empty_node = { 0 };
    bool found_pool_spot = false;

    for (size_t i = 0; i < this->pool_size; i++)
    {
        // check if this part of the array is zero, ready for new data
        if (memcmp(&this->pool[i], &empty_node, sizeof(Node)) == 0)
        {
            this->pool[i] = *node;
            found_pool_spot = true;
        }
    }

    if (!found_pool_spot)
    {
        LLOG_ERROR("Could not find a pool spot for node !%08x (%s)! Possible overflow?", node->id, node->short_name);
        LLOG_ERROR("Pool size: %u  Number of nodes: %u", this->pool_size, this->total_nodes);
        return false;
    }

    LLOG_DEBUG("Added node !%08x (%s) to NodeDB.", node->id, node->short_name);
    this->total_nodes++;
    return true;
}

bool _NodeDB::remove_node(uint32_t id)
{
    Node* possible_node = this->find_node(id);
    if (possible_node == nullptr)
    {
        LLOG_ERROR("Could not find node !%08x in NodeDB to remove. Ignoring request.", id);
        return false;
    }

    for (size_t i = 0; i < this->pool_size; i++)
    {
        if (this->pool[i].id == id)
            memset(&this->pool[i], 0, sizeof(Node));
    }

    return true;
}

bool _NodeDB::node_exists(uint32_t id)
{
    return this->find_node(id) != nullptr;
}

Node* _NodeDB::get_node(uint32_t id)
{
    Node* possible_node = this->find_node(id);
    if (possible_node == nullptr)
    {
        LLOG_ERROR("Could not find node !%08x in NodeDB. Ignoring query.", id);
        return nullptr;
    }

    // create a copy as to not leak the pool
    Node node = *this->find_node(id);
    return &node;
}