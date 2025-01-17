#include <tasks.h>
#include <gps.h>

uint64_t last_position_tx_time = 0;

void transmit_position_task()
{
    if ((millis() < (last_position_tx_time + GPS_TASK_TX_DELAY)) &&
        last_position_tx_time != 0)
        return;
    
    last_position_tx_time = millis();
    
    meshtastic_Data data_packet = meshtastic_Data_init_zero;
    data_packet.portnum = meshtastic_PortNum_POSITION_APP;
    data_packet.want_response = false;
    data_packet.has_bitfield = true;
    data_packet.bitfield = 0;

    meshtastic_Position position = meshtastic_Position_init_zero;
    
    if (GPS.satellites.isValid())
        position.sats_in_view = GPS.satellites.value();
    else
    {
        LLOG_DEBUG("No satellites in view. Have you tried turning it off and on again?");
        return;
    }

    if (GPS.location.isValid())
    {
        position.latitude_i = (int32_t)(GPS.location.lat() * 1e+7);
        position.longitude_i = (int32_t)(GPS.location.lng() * 1e+7);

        position.has_latitude_i = true;
        position.has_longitude_i = true;
    }
    else
    {
        // get out of function if we don't have location
        // don't want to be transmitting blank packet
        LLOG_DEBUG("No valid location, not transmitting position.");
        return;
    }

    uint32_t epoch = 0;
    if (GPS.time.isValid() && GPS.date.isValid())
    {
        epoch = convertToUnixTime(GPS.date.year(), GPS.date.month(), GPS.date.day(), GPS.time.hour(), GPS.time.minute(), GPS.time.second());
        position.time = epoch;
    }
    else
    {
        // also get out of function if we don't have time
        LLOG_DEBUG("No valid epoch, not transmitting position.");
        return;
    }

    if (GPS.altitude.isValid())
    {
        position.altitude = (int32_t)GPS.altitude.meters();
        position.has_altitude = true;
    }

    uint8_t position_buffer_raw[MAX_PAYLOAD_SIZE];

    // memory pointer shenanagens
    memset(position_buffer_raw, 0, MAX_PAYLOAD_SIZE);
    uint16_t position_buffer_length = 0;

    bool encode_status = pb_helper_encode(position_buffer_raw, MAX_PAYLOAD_SIZE, meshtastic_Position_fields, &position, &position_buffer_length);

    if (!encode_status)
    {
        LLOG_ERROR("Could not encode Position protobuf. Sad!");
        return;
    }

    meshtastic_Data_payload_t payload_buffer;
    payload_buffer.size = position_buffer_length;
    memcpy(payload_buffer.bytes, position_buffer_raw, payload_buffer.size);

    data_packet.payload = payload_buffer;


    uint8_t payload_buffer_raw[MAX_PAYLOAD_SIZE];
    uint16_t payload_length = 0;

    memset(payload_buffer_raw, 0, MAX_PAYLOAD_SIZE);

    encode_status = pb_helper_encode(payload_buffer_raw, MAX_PAYLOAD_SIZE, meshtastic_Data_fields, &data_packet, &payload_length);

    if (!encode_status)
    {
        LLOG_ERROR("Could not encode data packet protobuf. Sad!");
        return;
    }

    transmit_queue_record packet;
    MeshNetwork_PacketHeader header;
    MeshNetwork_PacketFlags flags;

    flags.hops = init_hops;
    flags.want_ack = false;
    flags.via_mqtt = false;
    flags.init_hops = init_hops;

    header.flags = encode_flags(&flags);
    header.destination = 0xFFFFFFFF;
    header.sender = device_id;
    header.id = generate_random_number(0xFFFFFF);
    header.hash = channels[0].hash;
    header.reserved = 0;

    AESResult encryption_test = aes_crypt_ctr_xcrypt(payload_buffer_raw, payload_length, header.id, header.sender, channels[0].key, channels[0].key_length);

    packet.payload_length = encryption_test.length;
    packet.payload = encryption_test.data;
    packet.header = header;
    packet.time_to_transmit = 0;

    transmit_queue.push(&packet);

    LLOG_INFO("Pushed position to transmit queue.");
}