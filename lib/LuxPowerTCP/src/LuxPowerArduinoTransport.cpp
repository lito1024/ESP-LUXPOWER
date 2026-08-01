#include "LuxPowerArduinoTransport.h"

#ifdef ARDUINO

LuxPowerArduinoTransport::LuxPowerArduinoTransport(Client &client)
    : client_(client)
{
}

bool LuxPowerArduinoTransport::connect(const char *host, uint16_t port)
{
    return client_.connect(host, port);
}

void LuxPowerArduinoTransport::disconnect()
{
    client_.stop();
}

bool LuxPowerArduinoTransport::connected() const
{
    return client_.connected();
}

size_t LuxPowerArduinoTransport::write(const uint8_t *data, size_t length)
{
    return client_.write(data, length);
}

void LuxPowerArduinoTransport::flush()
{
    client_.flush();
}

int LuxPowerArduinoTransport::available()
{
    return client_.available();
}

int LuxPowerArduinoTransport::read(uint8_t *buffer, size_t length)
{
    return client_.read(buffer, length);
}

#endif
