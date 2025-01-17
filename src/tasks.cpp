#include <tasks.h>
#include <airtime.h>

void transmitter_task()
{
    if (TX_ENABLED == false)
    {
        if (transmit_queue.is_empty() == false)
            transmit_queue.clean();
        
        return;
    }

    // we must believe that if we are transmitting and an operation flag is true,
    // then we must have finished sending a packet.
    if (transmission_state == RADIOLIB_ERR_NONE && operation_flag && transmitting)
    {
        LLOG_INFO("Finished sending packet.");
        radio.finishTransmit();
        radio.startReceive();
        transmitting = false;
        operation_flag = false;
        setLED(0, 0, 0);
    }
    else if (operation_flag && transmitting)
    {
        LLOG_ERROR("Error sending packet, error code: %d", transmission_state);
    }

    if (transmitting)
    {
        setLED(255, 0, 0);
        return;
    }

    if (transmit_queue.is_empty())
        return;

    if (millis() < (transmit_queue_last_tx_time + transmit_queue_delay))
        return;
    
    transmit_queue_last_tx_time = millis();

    transmit_queue_record record;
    transmit_queue.pop(&record);

    if (record.time_to_transmit == 0)
    {
        record.time_to_transmit = millis() + transmit_queue_delay;
        transmit_queue.push(&record);
        return;
    }

    if (millis() < record.time_to_transmit)
    {
        transmit_queue.push(&record);
        return;
    }

    LLOG_INFO("Transmitting packet from queue.");

    if (record.payload_length > (MAX_PAYLOAD_SIZE - sizeof(MeshNetwork_PacketHeader)))
    {
        LLOG_ERROR("Packet is smaller than a header, not transmitting.");
        return;
    }

    uint8_t transmit_buffer[MAX_PAYLOAD_SIZE];
    memset(transmit_buffer, 0, MAX_PAYLOAD_SIZE);

    uint32_t buffer_length = sizeof(MeshNetwork_PacketHeader) + record.payload_length;

    memcpy(transmit_buffer, &record.header, sizeof(MeshNetwork_PacketHeader));
    memcpy(transmit_buffer + sizeof(MeshNetwork_PacketHeader), record.payload, record.payload_length);

    is_scanning = true;
    int16_t channel_free = radio.scanChannel();
    if (channel_free != RADIOLIB_CHANNEL_FREE)
    {
        LLOG_ERROR("Unable to transmit packet due to channel being used. Error code: %d", channel_free);
        is_scanning = false;
        transmit_queue.push(&record);
        return;
    }

    transmission_state = radio.startTransmit(transmit_buffer, buffer_length);
    transmitting = true;
    is_scanning = false;
    airtime.add_tx_bytes(buffer_length);
}

void receive_task()
{
    // if we are transmitting, we should not be receiving anything.
    if (transmitting)
        return;

    // however, during the next iteration of loop, when we are no longer transmitting,
    // the operation flag is turned off. if we are not transmitting and if the operation
    // flag is true, then we must assume we have received a packet.
    if (!operation_flag)
        return;

    is_receiving = false;

    uint8_t data_buffer[MAX_PAYLOAD_SIZE];
    uint16_t packet_length = 0;
    memset(data_buffer, 0, MAX_PAYLOAD_SIZE);
    packet_length = (radio.getPacketLength() > MAX_PAYLOAD_SIZE ? MAX_PAYLOAD_SIZE : radio.getPacketLength());
    float rssi = radio.getRSSI();
    float snr = radio.getSNR();
    receive_state = radio.readData(data_buffer, MAX_PAYLOAD_SIZE);
    
    if (packet_length <= 0)
        return;

    airtime.add_rx_bytes(packet_length);

    if (receive_state == RADIOLIB_ERR_NONE || receive_state == RADIOLIB_ERR_CRC_MISMATCH)
    {
        did_receive = true;

        LLOG_DEBUG("Received packet.");

        if (packet_length <= sizeof(MeshNetwork_PacketHeader))
        {
            LLOG_WARNING("Got a payload length less than (or equal to) zero while decoding. Ignoring.");
            did_fail_to_decode = true;
            return;
        }
        if (receive_state == RADIOLIB_ERR_CRC_MISMATCH)
        {
            LLOG_WARNING("Packet had CRC mismatch, still attempting to decode.");
        }

        MeshNetwork_PacketHeader header;
        MeshNetwork_PacketFlags flags;

        // dogwater way to do this but whatever
        memcpy(&header, data_buffer, sizeof(MeshNetwork_PacketHeader));
        decode_flags(header.flags, &flags);

        if (flags.init_hops == flags.hops)
            LLOG_INFO("Length: %u  RSSI: %d  SNR: %0.2f  Direct connection", packet_length, (int32_t)rssi, snr);
        else
            LLOG_INFO("Length: %u  RSSI: %d  SNR: %0.2f  Hops away: %u/%u", packet_length, (int32_t)rssi, snr, flags.init_hops - flags.hops, flags.init_hops);

        if (header.destination != 0xFFFFFFFF)
            LLOG_INFO("From: !%08x  To: !%08x  Channel hash: %02x", header.sender, header.destination, header.hash);
        else
            LLOG_INFO("From: !%08x  Broadcast  Channel hash: %02x", header.sender, header.hash);

        uint8_t payload_buffer[MAX_PAYLOAD_SIZE];
        uint8_t payload_length = packet_length - sizeof(MeshNetwork_PacketHeader);

        // memory pointer shenanagens
        memset(payload_buffer, 0, MAX_PAYLOAD_SIZE);
        memcpy(payload_buffer, data_buffer + sizeof(MeshNetwork_PacketHeader), payload_length);

        Channel* channel = find_channel(header.hash);
        if (channel == nullptr)
        {
            LLOG_WARNING("Got channel hash %02x, but not in list of channels. Not decoding.", header.hash);
            did_receive = false;
            return;
        }

        if (header.sender == device_id)
        {
            LLOG_WARNING("Got packet from self, probably a bounce from a previous transmission. Ignoring.");
            did_receive = false;
            return;
        }

        AESResult decrypted = aes_crypt_ctr_xcrypt(payload_buffer, payload_length, header.id, header.sender, channel->key, channel->key_length);

        meshtastic_Data decoded = meshtastic_Data_init_zero;

        bool decode_status = pb_helper_decode(decrypted.data, decrypted.length, &meshtastic_Data_msg, &decoded);
        if (!decode_status)
        {
            LLOG_INFO("Hex: ");
            llog::default_serial_out->print("            ");
            for (size_t i = 0; i < packet_length; i++)
            {
                llog::default_serial_out->printf("%02x ", data_buffer[i]);
            }
            llog::default_serial_out->println();

            LLOG_INFO("Decrypted: ");
            llog::default_serial_out->print("            ");
            for (size_t i = 0; i < decrypted.length; i++)
            {
                llog::default_serial_out->printf("%02x ", decrypted.data[i]);
            }
            llog::default_serial_out->println();
            did_fail_to_decode = true;
            return;
        }

        did_fail_to_decode = !manage_decoded_packet(rssi, snr, &header, &flags, &decoded);

        // retransmit_packet(snr, &header, &flags, &decoded);
    }
    else
    {
        LLOG_ERROR("Failed to get packet, error code: %d", receive_state);
        did_fail_to_decode = true;
    }
}