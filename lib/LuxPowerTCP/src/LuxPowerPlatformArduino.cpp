#include "LuxPowerTransport.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

uint32_t luxPowerMillis()
{
    return millis();
}

void luxPowerDelay(uint32_t milliseconds)
{
    delay(milliseconds);
}

void luxPowerLog(const char *message)
{
    Serial.println(message);
}

void luxPowerLogf(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.print(buffer);
}

void luxPowerLogHex(const uint8_t *data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        if (data[i] < 0x10)
        {
            Serial.print('0');
        }

        Serial.print(data[i], HEX);
        Serial.print(' ');
    }

    Serial.println();
}

#endif
