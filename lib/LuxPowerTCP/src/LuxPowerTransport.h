#ifndef LUXPOWER_TRANSPORT_H
#define LUXPOWER_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

class LuxPowerTransport
{
public:
    virtual ~LuxPowerTransport() = default;

    virtual bool connect(const char *host, uint16_t port) = 0;
    virtual void disconnect() = 0;
    virtual bool connected() const = 0;
    virtual size_t write(const uint8_t *data, size_t length) = 0;
    virtual void flush() = 0;
    virtual int available() = 0;
    virtual int read(uint8_t *buffer, size_t length) = 0;

    int read()
    {
        uint8_t value = 0;
        const int count = read(&value, 1);
        return count == 1 ? value : -1;
    }
};

uint32_t luxPowerMillis();
void luxPowerDelay(uint32_t milliseconds);
void luxPowerLog(const char *message);
void luxPowerLogf(const char *format, ...);
void luxPowerLogHex(const uint8_t *data, size_t length);

#endif
