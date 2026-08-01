#ifndef LUXPOWER_TCP_H
#define LUXPOWER_TCP_H

#include <stddef.h>
#include <stdint.h>

#include "LuxPowerTransport.h"
#include "LuxPowerRegisters.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <Client.h>
#include "LuxPowerArduinoTransport.h"
#endif

enum class LuxPowerMetric : uint8_t
{
    Pv1EnergyTotal,
    FeedInGridEnabled,
    Pv1Power,
    Pv2Power,
    BatterySoc,
    BatteryChargePower,
    BatteryDischargePower
};

struct LuxPowerMetricValue
{
    enum class Type : uint8_t
    {
        Float,
        Boolean
    };

    Type type;
    float floatValue;
    bool boolValue;
};

struct LuxPowerMetricDescriptor
{
    LuxPowerMetric id;
    const char *name;
    const char *unit;
    LuxPowerMetricValue::Type type;
};

class LuxPowerTCP
{
public:
    explicit LuxPowerTCP(LuxPowerTransport &transport);
#ifdef ARDUINO
    explicit LuxPowerTCP(Client &client);
    ~LuxPowerTCP();

    void begin(IPAddress ip, uint16_t port, const char *dongleSerial, const char *inverterSerial);
#endif
    void begin(const char *host, uint16_t port, const char *dongleSerial, const char *inverterSerial);

    bool connect();
    void disconnect();
    bool connected() const;

    bool poll();
    void setInputPollInterval(uint32_t intervalMs);
    void setHoldingPollInterval(uint32_t intervalMs);

    bool readHoldingU16(uint16_t address);
    bool readInputU16(uint16_t address);
    bool readInputU32(uint16_t address);

    bool pv1EnergyTotal(float &kWh) const;
    bool feedInGridEnabled(bool &enabled) const;
    bool pv1Power(float &watts) const;
    bool pv2Power(float &watts) const;
    bool batterySoc(float &percent) const;
    bool batteryChargePower(float &watts) const;
    bool batteryDischargePower(float &watts) const;
    bool metricValue(LuxPowerMetric metric, LuxPowerMetricValue &value) const;
    bool metricDescriptor(LuxPowerMetric metric, LuxPowerMetricDescriptor &descriptor) const;

    uint16_t lastU16() const;
    uint32_t lastU32() const;
    const char *lastError() const;
    bool inputCacheValid() const;
    uint32_t inputCacheAge() const;
    bool holdingCacheValid() const;
    uint32_t holdingCacheAge() const;
    bool cachedInputU16(uint16_t address, uint16_t &value) const;
    bool cachedInputU32(uint16_t address, uint32_t &value) const;
    bool cachedHoldingU16(uint16_t address, uint16_t &value) const;
    bool cachedHoldingU32(uint16_t address, uint32_t &value) const;
#ifdef ARDUINO
    void dumpInputRegisters(Stream &out) const;
    void dumpHoldingRegisters(Stream &out) const;
    bool dumpChangedInputRegisters(Stream &out, uint16_t minimumDelta = 1);
    bool dumpChangedHoldingRegisters(Stream &out, uint16_t minimumDelta = 1);
#endif

private:
    struct RegisterCache
    {
        uint16_t input[125]{};
        uint16_t holding[125]{};
        bool inputValid = false;
        bool holdingValid = false;
        uint32_t inputUpdated = 0;
        uint32_t holdingUpdated = 0;

        void clear()
        {
            for (size_t i = 0; i < sizeof(input) / sizeof(input[0]); ++i)
            {
                input[i] = 0;
                holding[i] = 0;
            }

            inputValid = false;
            holdingValid = false;
            inputUpdated = 0;
            holdingUpdated = 0;
        }
    };

    bool discardInitialDongleData();
    bool consumeUnsolicitedFrames(uint32_t timeoutMs);
    bool readLuxFrame(uint8_t *buffer, size_t capacity, size_t &frameLength, uint32_t timeoutMs);
    size_t buildReadRequest(uint8_t *packet, size_t packetCapacity, uint16_t startRegister, uint16_t registerCount, uint8_t functionCode);
    uint16_t modbusCrc16(const uint8_t *data, size_t length);
    bool parseRegisterResponse(const uint8_t *packet, size_t length, uint16_t expectedRegister, uint8_t expectedFunction, uint16_t &value16);
    bool parseRegisterResponseU32(const uint8_t *packet, size_t length, uint16_t expectedRegister, uint8_t expectedFunction, uint32_t &value32);
    bool refreshInputBlock();
    bool refreshHoldingBlock();
    bool readRegister(uint16_t address, uint8_t functionCode, uint16_t &value16);
    bool readRegisterU32(uint16_t address, uint8_t functionCode, uint32_t &value32);

    LuxPowerTransport &transport_;
#ifdef ARDUINO
    LuxPowerArduinoTransport *ownedArduinoTransport_;
#endif
    char host_[64];
    uint16_t port_;
    const char *dongleSerial_;
    const char *inverterSerial_;
    const char *lastError_;
    uint16_t lastU16_;
    uint32_t lastU32_;
    RegisterCache cache_;
    uint16_t previousInput_[125]{};
    uint16_t previousHolding_[125]{};
    bool previousInputValid_ = false;
    bool previousHoldingValid_ = false;
    uint32_t inputPollInterval_;
    uint32_t holdingPollInterval_;
    uint32_t lastConnectAttemptMs_;
    uint8_t consecutiveResponseTimeouts_;

    static constexpr uint8_t MODBUS_FUNCTION_READ_HOLDING = 0x03;
    static constexpr uint8_t MODBUS_FUNCTION_READ_INPUT = 0x04;
    static constexpr uint16_t DEFAULT_TIMEOUT_MS = 3000;
    static constexpr uint16_t MATCHING_RESPONSE_TIMEOUT_MS = 3000;
    static constexpr uint16_t OVERALL_TIMEOUT_MS = 8000;
};

#endif
