#include <util.h>
#include <network.h>

uint32_t generate_random_number(uint32_t max)
{
    static uint32_t last_rand = 0;
    srand(radio.randomByte() + last_rand);
    last_rand = rand() % max;
    return last_rand;
}

void printnhex(uint8_t *data, size_t length, uint32_t address_offset)
{
    size_t address = 0;
    
    for (; address < length; address++)
    {
        if ((address % 16) == 0)
        {
            llog::default_serial_out->printf("\n%06x   ", address + address_offset);
        }
        
        if ((address % 4) == 0 && (address % 16) != 0)
        {
            llog::default_serial_out->printf(" ");
        }
        
        llog::default_serial_out->printf("%02x ", data[address]);
    }

    llog::default_serial_out->println();
}