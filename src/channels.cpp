#include <channels.h>
#include <string.h>
#include <util.h>
#include <llog.h>
#include <base64.hpp>

Channel channels[MAX_CHANNELS];
uint16_t num_channels = 0;

void add_channel(uint8_t* name, uint8_t name_length, uint8_t* key, uint8_t key_length)
{
    if (num_channels + 1 > MAX_CHANNELS)
    {
        LLOG_ERROR("Tried adding channel '%s' to list but list is already full! Max is %u channels.", name, MAX_CHANNELS);
        return;
    }

    if (name_length > MAX_CHANNEL_NAME)
    {
        LLOG_ERROR("Tried adding channel '%s' to list but name exceeds maximum length (which is %u)!", name, MAX_CHANNEL_NAME);
        return;
    }

    // we add 1 to the thing because of the \0 character
    // and honestly fuck the heap, we don't need that
    // FIXME: make this better in future version
    channels[num_channels].name = (uint8_t*)malloc(MAX_CHANNEL_NAME + 1);
    channels[num_channels].key = (uint8_t*)malloc(key_length);

    memcpy(channels[num_channels].name, name, name_length);
    channels[num_channels].name_length = name_length;
    memcpy(channels[num_channels].key, key, key_length);
    channels[num_channels].key_length = key_length;
    channels[num_channels].hash = calculate_channel_hash(channels[num_channels].name, channels[num_channels].name_length, channels[num_channels].key, channels[num_channels].key_length);

    LLOG_DEBUG("Added channel '%s' to list.", name);
    num_channels++;
}

void add_channel_b64(uint8_t* name, uint8_t name_length, uint8_t* key)
{
    uint8_t key_buffer[32];
    uint8_t key_length = 0;
    memset(key_buffer, 0, 32);

    key_length = decode_base64(key, strlen((const char*)key), key_buffer);

    add_channel(name, name_length, key_buffer, key_length);
}

void init_channels()
{
    memset(channels, 0, sizeof(Channel) * MAX_CHANNELS);    
}

uint8_t calculate_channel_hash(uint8_t* channel_name_ptr, uint8_t channel_name_length, uint8_t* key_ptr, uint8_t key_length)
{
    uint8_t name_hash = 0;
    for (uint8_t i = 0; i < channel_name_length; i++)
        name_hash ^= channel_name_ptr[i];
    
    uint8_t key_hash = 0;
    for (uint8_t i = 0; i < key_length; i++)
    {
        // LLOG_DEBUG("result: %02x  char: %02x", key_hash, key_ptr[i]);
        key_hash ^= key_ptr[i];
    }
    
    // LLOG_DEBUG("key_length: %u  name_hash: %02x  key_hash: %02x", key_length, name_hash, key_hash);

    return name_hash ^ key_hash;
}

Channel* find_channel(uint8_t hash)
{
    for (uint16_t i = 0; i < MAX_CHANNELS; i++)
    {
        if (channels[i].hash == hash && channels[i].key_length != 0)
            return &channels[i];
    }

    return nullptr;
}