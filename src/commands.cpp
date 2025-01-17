#include <llog.h>
#include <commands.h>
#include <config.h>
#include <channels.h>
#include <pb_helper.h>
#include <network.h>
#include <util.h>
#include <crypt.h>
#include <gps.h>
#include <battery.h>
#include <transmit_queue.h>
#include <memory.h>

#include "protobufs/mesh.pb.h"


uint32_t last_received_char_time = 0;
bool did_feed_buffer_to_parser = true;

char serial_buffer[MAX_SIZE];
bool serial_first_loop = false;

uint8_t test_command(uint8_t argc, char *argv[])
{
    LLOG_INFO("OK");
    
    return 0;
}

uint8_t show_warranty(uint8_t argc, char *argv[])
{
    LLOG_INFO("Section 15, GNU General Public License Version 3.");
    LLOG_INFO("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.");
    LLOG_INFO("EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER");
    LLOG_INFO("PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED");
    LLOG_INFO("OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF");
    LLOG_INFO("MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE");
    LLOG_INFO("QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE");
    LLOG_INFO("DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.");

    return 0;
}

uint8_t show_copyright(uint8_t argc, char *argv[])
{
    LLOG_INFO("Copyright (C) llamaking136 (llama@llamaking.net)  A.D. 2024");
    LLOG_INFO("");
    LLOG_INFO("This program is free software: you can redistribute it and/or modify");
    LLOG_INFO("it under the terms of the GNU General Public License as published by");
    LLOG_INFO("the Free Software Foundation, either version 3 of the License, or");
    LLOG_INFO("(at your option) any later version.");
    LLOG_INFO("");
    LLOG_INFO("This program is distributed in the hope that it will be useful,");
    LLOG_INFO("but WITHOUT ANY WARRANTY; without even the implied warranty of");
    LLOG_INFO("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
    LLOG_INFO("GNU General Public License for more details.");
    LLOG_INFO("");
    LLOG_INFO("You should have received a copy of the GNU General Public License");
    LLOG_INFO("along with this program.  If not, see <https://www.gnu.org/licenses/>.");
    LLOG_INFO("");
    LLOG_INFO("Meshtastic (R) is a registered trademark of Meshtastic LLC. Meshtastic");
    LLOG_INFO("software components are released under various licenses.");
    LLOG_INFO("Go to https://github.com/meshtastic and https://meshtastic.org for details.");
    LLOG_INFO("");
    LLOG_INFO("Glory to God in the highest, and on earth peace, good will toward men.");

    return 0;
}

void init_show_copyright()
{
    LLOG_INFO("Copyright (C) llamaking136 (llama@llamaking.net)  A.D. 2024");
    LLOG_INFO("");
    LLOG_INFO("This program comes with ABSOLUTELY NO WARRANTY; for details type `warranty'.");
    LLOG_INFO("This is free software, and you are welcome to redistribute it");
    LLOG_INFO("under certain conditions; type `copyright' for details.");
    LLOG_INFO("");
    LLOG_INFO("Meshtastic (R) is a registered trademark of Meshtastic LLC. Meshtastic");
    LLOG_INFO("software components are released under various licenses.");
    LLOG_INFO("Go to https://github.com/meshtastic and https://meshtastic.org for details.");
    LLOG_INFO("");
    LLOG_INFO("Glory to God in the highest, and on earth peace, good will toward men.");
}

uint8_t list_channels(uint8_t argc, char *argv[])
{
    for (uint16_t i = 0; i < MAX_CHANNELS; i++)
    {
        if (channels[i].name == nullptr)
            continue;
        
        if (channels[i].key == nullptr)
            LLOG_INFO("'%s'  hash %02x, unencrypted (no key)", channels[i].name, channels[i].hash);
        else
            LLOG_INFO("'%s'  hash %02x, key size %u", channels[i].name, channels[i].hash, channels[i].key_length * 8);
    }

    return 0;
}

uint8_t send_message(uint8_t argc, char *argv[])
{
    if (argc != 3)
    {
        LLOG_ERROR("usage: send_message <channel hash in hex> <message>");
        return 1;
    }

    // garbage code but whatever
    uint8_t hash = (uint8_t)strtol(argv[1], NULL, 16);
    if (hash == 0)
    {
        LLOG_ERROR("strtol returned 0, exiting.");
        return 1;
    }

    LLOG_INFO("Channel hash: %02x", hash);

    Channel* channel = find_channel(hash);
    if (channel == nullptr)
    {
        LLOG_ERROR("Did not find a channel by that hash, exiting.");
        return 1;
    }

    LLOG_INFO("Channel name: %s", channel->name);

    uint8_t buffer[MAX_PAYLOAD_SIZE];
    memset(buffer, 0, MAX_PAYLOAD_SIZE);

    meshtastic_Data data_packet = meshtastic_Data_init_zero;
    data_packet.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
    data_packet.want_response = false;
    data_packet.has_bitfield = true;
    data_packet.bitfield = 0;

    meshtastic_Data_payload_t payload;
    payload.size = strnlen(argv[2], MAX_SIZE);
    memcpy(payload.bytes, argv[2], payload.size);

    data_packet.payload = payload;

    uint16_t encoded_length = 0;

    bool encode_status = pb_helper_encode(buffer, MAX_PAYLOAD_SIZE, meshtastic_Data_fields, &data_packet, &encoded_length);
    if (!encode_status)
    {
        LLOG_ERROR("Could not encode text message protobuf. Sad!");
        return 1;
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
    header.hash = channel->hash;
    header.reserved = 0;

    AESResult encryption_test = aes_crypt_ctr_xcrypt(buffer, encoded_length, header.id, header.sender, channel->key, channel->key_length);

    packet.payload_length = encryption_test.length;
    packet.payload = encryption_test.data;
    packet.header = header;
    packet.time_to_transmit = 0;

    transmit_queue.push(&packet);

    LLOG_INFO("Pushed text message to transmit queue.");

    return 0;
}

uint8_t gps_info(uint8_t argc, char *argv[])
{
    print_gps_task(true);

    return 0;
}

uint8_t battery_info(uint8_t argc, char *argv[])
{
    battery.get_battery_voltage();
    LLOG_INFO("Battery voltage: %u  %u%%", battery.get_voltage(), battery.get_percentage());

    return 0;
}

uint8_t memory_info(uint8_t argc, char *argv[])
{
    char *shart = (char*)malloc(35);

    int free_memory = memory_manager.get_free_memory();
    LLOG_INFO("Free memory: %d", free_memory);

    return 0;
}

Command __test_command(test_command, "test");
Command __warranty(show_warranty, "warranty");
Command __copyright(show_copyright, "copyright");
Command __send_message(send_message, "send_message");
Command __list_channels(list_channels, "list_channels");
Command __gps_info(gps_info, "gps_info");
Command __battery_info(battery_info, "battery_info");
Command __memory_info(memory_info, "memory_info");

Command command_lookup_table[] = {
    __test_command,
    __warranty,
    __copyright,
    __send_message,
    __list_channels,
    __gps_info,
    __battery_info,
    __memory_info
};

unsigned int parse_command_line(const char *input, char *argv[])
{
    if (strnlen(input, MAX_SIZE) == 0)
    {
        return 0;
    }
    
    char raw_buffer[MAX_SIZE];
    char *buffer = raw_buffer;
    size_t buffer_location = 0;

    memset(buffer, 0, MAX_SIZE);

    bool in_single_string = false;
    bool in_double_string = false;
    unsigned int argc = 0;
    
    for (unsigned int i = 0; i < strnlen(input, MAX_SIZE); i++)
    {
        if (input[i] == '\'')
            in_single_string = !in_single_string;
        
        if (input[i] == '"')
            in_double_string = !in_double_string;
    
        if (input[i] == ' ' && argc < MAX_ARGC && !in_single_string && !in_double_string)
        {
            buffer[buffer_location++] = '\0';
            strncpy(argv[argc++], buffer, strnlen(buffer, MAX_SIZE));
            memset(buffer, 0, MAX_SIZE);
            buffer_location = 0;
            continue;
        }

        if (input[i] != '\'' && input[i] != '"')
            buffer[buffer_location++] = input[i];
    }
    
    strncpy(argv[argc++], buffer, strnlen(buffer, MAX_SIZE));

    return argc;
}

void handle_serial_string(char *buffer)
{
    // currently this uses 16kb of stack memory
    // wtf was the decision behind this
    char argv[MAX_SIZE][MAX_ARGC];
    memset(argv, 0, MAX_SIZE * MAX_ARGC);

    char *ptr_argv[MAX_ARGC];

    // these next four lines of code took all fucking day to fix this shit code
    // glory be to God in the highest
    for (size_t i = 0; i < MAX_ARGC; i++)
    {
        ptr_argv[i] = argv[i];
    }

    unsigned int argc = parse_command_line(buffer, ptr_argv);
    bool found_command = false;

    // loop through all commands and find the right one
    for (unsigned int i = 0; i < (sizeof(command_lookup_table) / sizeof(command_lookup_table[0])); i++)
    {
        if (strcmp(command_lookup_table[i].name, argv[0]) == 0)
        {
            found_command = true;
            unsigned int return_code = command_lookup_table[i].function(argc, ptr_argv);

            if (return_code > 0)
                LLOG_DEBUG("%s finished with code %d", argv[0], return_code);
        }
    }

    if (!found_command)
    {
        LLOG_WARNING("%s: command not found", argv[0]);
    }
}

void read_serial_buffer()
{
    if (serial_first_loop == false)
    {
        memset(serial_buffer, 0, MAX_SIZE);
        serial_first_loop = true;
    }

    char *buffer = serial_buffer;
    size_t buffer_length = 0;

    char is_available = llog::default_serial_in->available();
    char char_buffer = '\0';

    bool is_gps = false;

    if (is_available > 0)
    {
        did_feed_buffer_to_parser = false;

        while (llog::default_serial_in->available() > 0 && buffer_length <= MAX_SIZE)
        {
            last_received_char_time = millis();

            char_buffer = llog::default_serial_in->read();

            if (buffer_length >= 2 &&
                buffer[buffer_length - 2] == '$' &&
                buffer[buffer_length - 1] == 'G' &&
                (char_buffer == 'P' || char_buffer == 'p'))
            {
                is_gps = true;
            }

            // LLOG_DEBUG("%c  %u", char_buffer, buffer_length);

            // why
            // fuck
            delay(1);

            // add character to buffer
            if (char_buffer != SERIAL_COMMAND_TERMINATOR && is_gps == false)
            {
                buffer[buffer_length++] = char_buffer;
            }

            // if gps, encode character and add to buffer
            if (is_gps)
            {
                GPS.encode(char_buffer);
                buffer[buffer_length++] = char_buffer;
            }

            // if end of gps message, clear gps flag and die
            if (is_gps &&
                char_buffer == '\n')
            {
                is_gps = false;
                memset(serial_buffer, 0, MAX_SIZE);
                buffer_length = 0;
            }
        }
    }

    // if the timeout has been reached or if the last character was the line terminator,
    // we should probably parse the buffer
    if ((millis() >= last_received_char_time + SERIAL_COMMAND_TIMEOUT || 
        char_buffer == SERIAL_COMMAND_TERMINATOR) &&
        did_feed_buffer_to_parser == false)
    {
        handle_serial_string(buffer);
        did_feed_buffer_to_parser = true;
        memset(serial_buffer, 0, MAX_SIZE);
    }
}