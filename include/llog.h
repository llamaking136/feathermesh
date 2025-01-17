#pragma once

// llog stands for llama's logging bullshit

#include <Arduino.h>
#include <stdarg.h>

// number of bytes that define...something
// i cant remember, too many things rely on this
#define MAX_SIZE 256

#define LLOG_DEBUG(fmt, ...) printf_ffl(llog::LogLevel::DEBUG, __FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LLOG_INFO(fmt, ...) printf_ffl(llog::LogLevel::INFO, __FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LLOG_WARNING(fmt, ...) printf_ffl(llog::LogLevel::WARNING, __FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LLOG_ERROR(fmt, ...) printf_ffl(llog::LogLevel::ERROR, __FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LLOG_CRITICAL(fmt, ...) printf_ffl(llog::LogLevel::CRITICAL, __FUNCTION__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

namespace llog
{
    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    extern LogLevel default_loglevel;
    extern HardwareSerial *default_serial_out;
    extern HardwareSerial *default_serial_in;

    const char *get_str_from_level(LogLevel);

    void print_format(const char *, bool, LogLevel = default_loglevel);
    
    void printf(LogLevel, bool, const char *, ...);
    
    void printf_ffl(LogLevel, const char *, const char *, int, const char *, ...);
};

uint32_t string_to_hex(char *ptr, size_t len);



typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);
extern "C" int _vsnprintf(out_fct_type out,char* buffer, const size_t maxlen, const char* format, va_list va);

// internal buffer output
static inline void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen);