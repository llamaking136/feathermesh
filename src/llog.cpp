#include <llog.h>
#include <config.h>

// llog stands for llama's logging bullshit

llog::LogLevel llog::default_loglevel = llog::LogLevel::DEBUG;
HardwareSerial *llog::default_serial_out = &Serial;
HardwareSerial *llog::default_serial_in = &Serial;

const char *fmt_string = "[%s] ";

const char * llog::get_str_from_level(llog::LogLevel level)
{
    switch (level)
    {
        case llog::LogLevel::DEBUG:
            return COMMAND_OUTPUT_COLOR ? "\e[1;94mDEBUG   \e[0m" : "DEBUG   ";
            break;
        case llog::LogLevel::INFO:
            return COMMAND_OUTPUT_COLOR ? "\e[1;97mINFO    \e[0m" : "INFO    ";
            break;
        case llog::LogLevel::WARNING:
            return COMMAND_OUTPUT_COLOR ? "\e[1;38:5:208:0mWARNING \e[0m" : "WARNING ";
            break;
        case llog::LogLevel::ERROR:
            return COMMAND_OUTPUT_COLOR ? "\e[1;31mERROR   \e[0m" : "ERROR   ";
            break;
        case llog::LogLevel::CRITICAL:
            return COMMAND_OUTPUT_COLOR ? "\e[1;91mCRITICAL\e[0m" : "CRITICAL";
            break;
        default:
            return COMMAND_OUTPUT_COLOR ? "\e[1;37mUNKNOWN \e[0m" : "UNKNOWN ";
            break;
    }
}

const char * get_color_from_level(llog::LogLevel level)
{
    switch (level)
    {
        case llog::LogLevel::DEBUG:
            return "\e[1;94m";
            break;
        case llog::LogLevel::INFO:
            return "\e[1;97m";
            break;
        case llog::LogLevel::WARNING:
            return "\e[1;38:5:208:0m";
            break;
        case llog::LogLevel::ERROR:
            return "\e[0;31m";
            break;
        case llog::LogLevel::CRITICAL:
            return "\e[1;91m";
            break;
        default:
            return "\e[0;37m";
            break;
    }
}

const char * get_clear_color()
{
    return "\e[0m";
}

uint32_t string_to_hex(char *ptr, size_t len)
{
	uint32_t result = 0;
	
	if (len < 3)
	{
		return 0;
	}
	
	for (uint32_t i = 0; i < len; i++)
	{
		if (ptr[i] == '0' && (ptr[i + 1] == 'x' || ptr[i + 1] == 'X'))
		{
			i++;
			continue;
		}
		
		if (ptr[i] >= '0' && ptr[i] <= '9')
		{
			result += ptr[i] - '0';
		}
		else if (ptr[i] >= 'a' && ptr[i] <= 'f')
		{
			result += ptr[i] - ('a' - 10);
		}
		else if (ptr[i] >= 'A' && ptr[i] <= 'F')
		{
			result += ptr[i] - ('A' - 10);
		}
		else
		{
			return 0;
		}
		
		if (i + 1 < len)
		{
			result *= 16;
		}
	}
	
	return result;
}

// typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);
// extern "C" int _vsnprintf(out_fct_type out,char* buffer, const size_t maxlen, const char* format, va_list va);

// internal buffer output
static inline void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen)
{
  if (idx < maxlen) {
    ((char*)buffer)[idx] = character;
  }
}

void llog::print_format(const char *fmt, bool newline, llog::LogLevel level)
{
    default_serial_out->printf(fmt_string, llog::get_str_from_level(level));
    
    default_serial_out->print(fmt);

    if (newline)
        default_serial_out->println();
}

void llog::printf(llog::LogLevel level, bool newline, const char *fmt, ...)
{
    char data[MAX_SIZE];
    char *ptr = data;

    va_list args;
    va_start(args, fmt);

    _vsnprintf(_out_buffer, ptr, MAX_SIZE, fmt, args);

    llog::print_format(ptr, newline, level);

    va_end(args);
}

void llog::printf_ffl(llog::LogLevel level, const char *function, const char *filename, int line, const char *fmt, ...)
{
    if (SHOW_COMMAND_OUTPUT)
    {
        if (COMMAND_OUTPUT_COLOR)
            llog::printf(level, false, "(\e[0;96m%s\e[0m:\e[0;92m%s\e[0:\e[0;94m:%d\e[0m) ", filename, function, line);
        else
            llog::printf(level, false, "(%s:%s:%d) ", filename, function, line);
    }
    else
        llog::printf(level, false, " ");
    
    char data[MAX_SIZE];
    char *ptr = data;

    va_list args;
    va_start(args, fmt);

    _vsnprintf(_out_buffer, ptr, MAX_SIZE, fmt, args);

    if (COMMAND_OUTPUT_COLOR)
        default_serial_out->print(get_color_from_level(level));

    default_serial_out->print(ptr);

    if (COMMAND_OUTPUT_COLOR)
        default_serial_out->print(get_clear_color());
    
    default_serial_out->println();

    va_end(args);
}