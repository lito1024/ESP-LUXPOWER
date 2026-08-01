#ifndef LUXPOWER_ARDUINO_TRANSPORT_H
#define LUXPOWER_ARDUINO_TRANSPORT_H

#include "LuxPowerTransport.h"

#ifdef ARDUINO
#include <Client.h>

class LuxPowerArduinoTransport : public LuxPowerTransport
{
public:
    explicit LuxPowerArduinoTransport(Client &client);

    bool connect(const char *host, uint16_t port) override;
    void disconnect() override;
    bool connected() const override;
    size_t write(const uint8_t *data, size_t length) override;
    void flush() override;
    int available() override;
    int read(uint8_t *buffer, size_t length) override;

private:
    Client &client_;
};
#endif

#endif
