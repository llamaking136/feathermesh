#pragma once

#include <stdint.h>

#define MAX_ARGC 16
#define SERIAL_COMMAND_TERMINATOR '\r'
#define SERIAL_COMMAND_TIMEOUT 100

struct Command
{
    uint8_t (*function)(uint8_t, char **);
    const char *name;

    Command(uint8_t (*function)(uint8_t, char **), const char *name)
    {
        this->function = function;
        this->name = name;
    }
    ~Command() {}
};

extern Command command_lookup_table[];
extern Command __test_command;

unsigned int parse_command_line(const char *, char *[]);
void read_serial_buffer();

void init_show_copyright();
uint8_t show_copyright(uint8_t, char *[]);