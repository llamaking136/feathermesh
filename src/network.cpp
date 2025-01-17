#include <network.h>
#include <util.h>
#include <config.h>
#include <pb_helper.h>
#include <llog.h>
#include <airtime.h>
#include <transmit_queue.h>
#include <crypt.h>
#include <channels.h>

uint8_t* transmit_buffer;

uint16_t calculate_packet_delay(float snr)
{
    return (uint16_t)floor(6500.0f * pow(0.91f, 20.0f + snr) + (15.0f * SLOT_TIME) + (10.0f * airtime.get_airtime()) + (15.0f * (float)generate_random_number(35)));
}

uint8_t encode_flags(MeshNetwork_PacketFlags* flags)
{
    uint8_t compressed = 0;

    compressed |= (0b11100000 & (flags->init_hops << 5));
    compressed |= (0b00010000 & (flags->via_mqtt << 4));
    compressed |= (0b00001000 & (flags->want_ack << 3));
    compressed |= (0b00000111 & (flags->hops));

    return compressed;
}

void decode_flags(uint8_t compressed, MeshNetwork_PacketFlags* flags)
{
    flags->init_hops = 0b00000111 & (compressed >> 5);
    flags->via_mqtt = 0b00000001 & (compressed >> 4);
    flags->want_ack  = 0b00000001 & (compressed >> 3);
    flags->hops = 0b00000111 & compressed;
}

// internal buffer output
static inline void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen)
{
  if (idx < maxlen) {
    ((char*)buffer)[idx] = character;
  }
}
int __wrap_sprintf(char* buffer, const char* format, ...)
{
  va_list va;
  va_start(va, format);
  const int ret = _vsnprintf(_out_buffer, buffer, (size_t)-1, format, va);
  va_end(va);
  return ret;
}

void retransmit_packet_status(int32_t rssi, float snr, uint8_t hops, uint8_t init_hops)
{
    uint8_t buffer[MAX_PAYLOAD_SIZE];
    memset(buffer, 0, MAX_PAYLOAD_SIZE);

    if (init_hops - hops != 0)
        __wrap_sprintf((char*)buffer, "RSSI: %d\nSNR: %0.2f\nHops away: %u/%u", rssi, snr, init_hops - hops, init_hops);
    else
        __wrap_sprintf((char*)buffer, "RSSI: %d\nSNR: %0.2f\nDirect connection", rssi, snr);

    meshtastic_Data data_packet = meshtastic_Data_init_zero;
    
    data_packet.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
    data_packet.want_response = false;
    data_packet.has_bitfield = true;
    data_packet.bitfield = 0;

    meshtastic_Data_payload_t payload;
    payload.size = strnlen((const char*)buffer, MAX_SIZE);
    memcpy(payload.bytes, buffer, payload.size);

    data_packet.payload = payload;
    uint16_t encoded_length = 0;

    bool encode_status = pb_helper_encode(buffer, MAX_PAYLOAD_SIZE, meshtastic_Data_fields, &data_packet, &encoded_length);
    if (!encode_status)
    {
        return;
    }
    transmit_queue_record packet;
    MeshNetwork_PacketHeader header;
    MeshNetwork_PacketFlags flags;

    Channel* channel = find_channel(0xc6);
    if (channel == nullptr)
    {
        LLOG_ERROR("Couldn't find channel to broadcast telemetry to.");
        return;
    }

    flags.hops = init_hops;
    flags.want_ack = false;
    flags.via_mqtt = false;
    flags.init_hops = init_hops;

    header.flags = encode_flags(&flags);
    header.destination = 0xFFFFFFFF;
    header.sender = device_id;
    header.id = generate_random_number(0xFFFFFFFF);
    header.hash = channel->hash;
    header.reserved = 0;

    AESResult encryption_test = aes_crypt_ctr_xcrypt(buffer, encoded_length, header.id, header.sender, channel->key, channel->key_length);

    packet.payload_length = encryption_test.length;
    packet.payload = encryption_test.data;
    packet.header = header;
    packet.time_to_transmit = millis() + 1000;

    transmit_queue.push(&packet);
}

bool manage_decoded_packet(int32_t rssi, float snr, MeshNetwork_PacketHeader* header, MeshNetwork_PacketFlags* flags, meshtastic_Data* decoded)
{
    LLOG_INFO("Portnum: %d", decoded->portnum);
        
    if (decoded->portnum == meshtastic_PortNum_TEXT_MESSAGE_APP)
    {
        LLOG_INFO("Payload: '%s'", decoded->payload.bytes);

        const char* test_string = "among us";
        if (strncmp((const char *)decoded->payload.bytes, test_string, strlen(test_string)) == 0)
        {
            if (header->sender == device_id)
                return false;
            
            if (header->hash != 0xc6)
                return false;

            retransmit_packet_status(rssi, snr, flags->hops, flags->init_hops);
        }
    }
    else if (decoded->portnum == meshtastic_PortNum_NODEINFO_APP)
    {
        meshtastic_User nodeinfo_user = meshtastic_User_init_zero;

        bool decode_status = pb_helper_decode(decoded->payload.bytes, decoded->payload.size, &meshtastic_User_msg, &nodeinfo_user);
        if (!decode_status)
        {
            return false;
        }
        
        LLOG_INFO("Long name: %s", nodeinfo_user.long_name);
        LLOG_INFO("Short name: %s", nodeinfo_user.short_name);
    }
    else if (decoded->portnum == meshtastic_PortNum_POSITION_APP)
    {
        meshtastic_Position position = meshtastic_Position_init_zero;

        bool decode_status = pb_helper_decode(decoded->payload.bytes, decoded->payload.size, &meshtastic_Position_msg, &position);
        if (!decode_status)
        {
            return false;
        }

        LLOG_INFO("Satellites: %u  HDOP: %u  Accuracy: %u", position.sats_in_view, position.HDOP, position.gps_accuracy);
        LLOG_INFO("Epoch: %u", position.time);
        if (!REDACT_POSITIONS)
        {
            if (position.has_latitude_i && position.has_longitude_i)
            {
                double latitude = (double)position.latitude_i * 1e-7;
                double longitude = (double)position.longitude_i * 1e-7;

                LLOG_INFO("Latitude: %f  Longitude: %f", latitude, longitude);
            }
            if (position.has_altitude)
            {
                LLOG_INFO("Altitude: %dm", position.altitude);
            }
        }
    }
    else if (decoded->portnum == meshtastic_PortNum_TELEMETRY_APP)
    {
        meshtastic_Telemetry telemetry = meshtastic_Telemetry_init_zero;

        bool decode_status = pb_helper_decode(decoded->payload.bytes, decoded->payload.size, &meshtastic_Telemetry_msg, &telemetry);
        if (!decode_status)
        {
            return false;
        }

        LLOG_INFO("Epoch: %u", telemetry.time);
        if (telemetry.which_variant == meshtastic_Telemetry_device_metrics_tag)
        {
            meshtastic_DeviceMetrics metrics = telemetry.variant.device_metrics;

            if (metrics.has_uptime_seconds)
                LLOG_INFO("Uptime: %us", metrics.uptime_seconds);
            if (metrics.has_battery_level)
                LLOG_INFO("Battery level: %u%%", metrics.battery_level);
            if (metrics.has_voltage)
                LLOG_INFO("Voltage: %0.1fv", metrics.voltage);
            if (metrics.has_channel_utilization)
                LLOG_INFO("Channel utilization: %d%%", metrics.channel_utilization);
            if (metrics.has_air_util_tx)
                LLOG_INFO("TX airtime: %d%%", metrics.air_util_tx);
        }
        else if (telemetry.which_variant == meshtastic_Telemetry_environment_metrics_tag)
        {
            meshtastic_EnvironmentMetrics metrics = telemetry.variant.environment_metrics;

            if (metrics.has_temperature)
                LLOG_INFO("Temperature: %0.1fc", metrics.temperature);
            if (metrics.has_relative_humidity)
                LLOG_INFO("Humidity: %d%%", (int32_t)metrics.relative_humidity);
            if (metrics.has_barometric_pressure)
                LLOG_INFO("Pressure: %0.2f hPA", metrics.barometric_pressure);
            if (metrics.has_voltage)
                LLOG_INFO("Voltage: %0.1fv", metrics.voltage);
            if (metrics.has_current)
                LLOG_INFO("Current: %0.1fmA", metrics.current); // not sure what unit this is in, assuming mA
            if (metrics.has_wind_direction)
                LLOG_INFO("Wind direction: %u degrees", metrics.wind_direction);
            if (metrics.has_wind_speed)
                LLOG_INFO("Wind speed: %0.2f m/s", metrics.wind_speed);
            if (metrics.has_wind_gust)
                LLOG_INFO("Wind gust: %0.2f m/s", metrics.wind_gust);
        }
        else
            LLOG_INFO("Telemetry type: %d", telemetry.which_variant);
    }

    return true;
}

void retransmit_packet(float snr, MeshNetwork_PacketHeader* header, MeshNetwork_PacketFlags* flags, meshtastic_Data* decoded)
{
    if (header->destination == device_id)
    {
        LLOG_ERROR("Trying to retransmit packet towards self, bailing.");
        return;
    }

    if ((int8_t)flags->hops - 1 <= 0)
    {
        LLOG_WARNING("Trying to retransmit packet with %d hops remaining, ignoring.", (int8_t)flags->hops - 1);
        return;
    }

    meshtastic_Data data_packet = meshtastic_Data_init_zero;
    uint8_t buffer[MAX_PAYLOAD_SIZE];
    memset(buffer, 0, MAX_PAYLOAD_SIZE);
    uint16_t encoded_length = 0;

    bool encode_status = pb_helper_encode(buffer, MAX_PAYLOAD_SIZE, meshtastic_Data_fields, &data_packet, &encoded_length);
    if (!encode_status)
    {
        return;
    }

    transmit_queue_record packet;

    packet.payload_length = encoded_length;
    packet.payload = buffer;
    packet.header = *header;
    packet.time_to_transmit = millis() + calculate_packet_delay(snr);

    transmit_queue.push(&packet);

    LLOG_DEBUG("Pushed retransmission to queue.");
}