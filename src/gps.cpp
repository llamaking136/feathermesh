#include <gps.h>
#include <time.h>
#include <config.h>

TinyGPSPlus GPS;
softSerial GPSSerial(GPS_tx, GPS_rx);

bool did_print_no_rx_from_gps = false;

uint64_t last_gps_print_time = 0;
uint32_t gps_print_time_delay = 15 * 60 * 1000;

void init_gps()
{
    GPSSerial.begin(9600);
    LLOG_INFO("GPS serial initialized.");
}

void read_gps_buffer()
{
    if (millis() > 5000 && GPS.charsProcessed() < 10 && !did_print_no_rx_from_gps)
    {
        LLOG_CRITICAL("No RX from GPS, been 5 seconds since boot! Check wiring?");
        did_print_no_rx_from_gps = true;
    }

    if (GPSSerial.available() > 0)
    {
        char byte_buffer = GPSSerial.read();

        bool status = GPS.encode(byte_buffer);

        if (status == false) return;

        // if (GPS.location.isValid())
        // {
        //     Serial.printf("GPS latitude: %f  GPS longitude: %f\n", GPS.location.lat(), GPS.location.lng());
        //     Serial.flush();
        // }
    }
}

// stolen from chatcbt
time_t convertToUnixTime(int year, int month, int day, int hour, int minute, int second)
{
    // Adjust for Unix time starting from 1970
    struct tm timeinfo;
    timeinfo.tm_year = year - 1900; // Year since 1900
    timeinfo.tm_mon = month - 1;     // Month from 0 to 11
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    timeinfo.tm_isdst = 0; // Not considering daylight saving time

    return mktime(&timeinfo);
}

void print_gps_task(bool just_do_it = false)
{
    if (millis() < (last_gps_print_time + gps_print_time_delay) &&
        just_do_it == false)
        return;

    if (just_do_it == false)
        last_gps_print_time = millis();

    if (GPS.charsProcessed() < 10)
    {
        LLOG_INFO("No GPS device found.");
        return;
    }

    if (GPS.satellites.isValid())
    {
        LLOG_INFO("GPS Satellites: %u", GPS.satellites.value());
    }

    if (GPS.time.isValid() && GPS.date.isValid())
    {
        LLOG_INFO("GPS Time: %02u:%02u:%02u  GPS Date: %u/%02u/%02u", GPS.time.hour(), GPS.time.minute(), GPS.time.second(), GPS.date.year(), GPS.date.month(), GPS.date.day());
    }

    if (!REDACT_POSITIONS)
    {
        if (GPS.location.isValid())
        {
            LLOG_INFO("GPS Latitude: %f  GPS Longitude: %f", GPS.location.lat(), GPS.location.lng());
        }

        if (GPS.altitude.isValid())
        {
            LLOG_INFO("GPS Altitude: %0.1fm  Horizontal accuracy: %0.2fm", GPS.altitude.meters(), GPS.hdop.value() / 100.0);
        }
    }
}