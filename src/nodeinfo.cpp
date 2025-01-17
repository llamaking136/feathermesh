#include <tasks.h>

void transmit_nodeinfo_task()
{
    if ((millis() < (transmit_node_id_time + transmit_node_id_delay)) &&
        transmit_node_id_time != 0)
        return;
    
    transmit_node_id_time = millis();
    
    meshtastic_Data data_packet = meshtastic_Data_init_zero;
    data_packet.portnum = meshtastic_PortNum_NODEINFO_APP;
    data_packet.want_response = false;
    data_packet.has_bitfield = true;
    data_packet.bitfield = 0;
    
    meshtastic_User nodeinfo_user = meshtastic_User_init_zero;
    nodeinfo_user.hw_model = (meshtastic_HardwareModel)device_hardware_id;
    memcpy(nodeinfo_user.id, device_str_id, strlen((const char*)device_str_id));
    memcpy(nodeinfo_user.macaddr, device_mac_address, 6);
    memcpy(nodeinfo_user.long_name, device_long_name, strlen(device_long_name));
    memcpy(nodeinfo_user.short_name, device_short_name, strlen(device_short_name));
    nodeinfo_user.role = meshtastic_Config_DeviceConfig_Role_CLIENT;
    nodeinfo_user.is_licensed = false;

    nodeinfo_user.public_key.bytes[0] = 0;
    nodeinfo_user.public_key.size = 0;

    uint8_t nodeinfo_buffer_raw[MAX_PAYLOAD_SIZE];

    // memory pointer shenanagens
    memset(nodeinfo_buffer_raw, 0, MAX_PAYLOAD_SIZE);
    uint16_t nodeinfo_buffer_length = 0;

    bool encode_status = pb_helper_encode(nodeinfo_buffer_raw, MAX_PAYLOAD_SIZE, meshtastic_User_fields, &nodeinfo_user, &nodeinfo_buffer_length);

    if (!encode_status)
    {
        LLOG_ERROR("Could not encode User protobuf. Sad!");
        return;
    }

    meshtastic_Data_payload_t payload_buffer;
    payload_buffer.size = nodeinfo_buffer_length;
    memcpy(payload_buffer.bytes, nodeinfo_buffer_raw, payload_buffer.size);

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
    header.id = generate_random_number(0xFFFFFFFF);
    header.hash = channels[0].hash;
    header.reserved = 0;

    AESResult encryption_test = aes_crypt_ctr_xcrypt(payload_buffer_raw, payload_length, header.id, header.sender, channels[0].key, channels[0].key_length);

    packet.payload_length = encryption_test.length;
    packet.payload = encryption_test.data;
    packet.header = header;
    packet.time_to_transmit = 0;

    transmit_queue.push(&packet);

    LLOG_INFO("Pushed nodeinfo to transmit queue.");
}