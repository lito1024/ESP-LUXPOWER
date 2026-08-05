#include "LuxPowerTCP.h"

#include <string.h>

namespace
{
constexpr uint16_t REGISTER_COUNT = 2;
constexpr LuxPowerRegisterDescriptor kVerifiedRegisters[] = {
    {"PV1 Energy Total", "kWh", LuxPowerRegisterSpace::Input, LuxPowerRegisterValueType::U32, 40, 0, 0.1f},
    {"Feed-In Grid Enabled", "", LuxPowerRegisterSpace::Holding, LuxPowerRegisterValueType::Bit, 22, 15, 1.0f},
    {"PV1 Power", "W", LuxPowerRegisterSpace::Input, LuxPowerRegisterValueType::U16, 7, 0, 1.0f},
    {"PV2 Power", "W", LuxPowerRegisterSpace::Input, LuxPowerRegisterValueType::U16, 8, 0, 1.0f},
    {"Battery SOC", "%", LuxPowerRegisterSpace::Input, LuxPowerRegisterValueType::U16, 5, 0, 1.0f},
    {"Battery Charge Power", "W", LuxPowerRegisterSpace::Input, LuxPowerRegisterValueType::U16, 10, 0, 1.0f},
    {"Battery Discharge Power", "W", LuxPowerRegisterSpace::Input, LuxPowerRegisterValueType::U16, 11, 0, 1.0f}
};

bool lookupMetricDescriptor(LuxPowerMetric metric, LuxPowerRegisterDescriptor &descriptor)
{
    switch (metric)
    {
        case LuxPowerMetric::Pv1EnergyTotal:
            return luxPowerRegisterDescriptor(LuxPowerRegisterId::Pv1EnergyTotal, descriptor);

        case LuxPowerMetric::FeedInGridEnabled:
            return luxPowerRegisterDescriptor(LuxPowerRegisterId::FeedInGridEnabled, descriptor);

        case LuxPowerMetric::Pv1Power:
            return luxPowerRegisterDescriptor(LuxPowerRegisterId::Pv1Power, descriptor);

        case LuxPowerMetric::Pv2Power:
            return luxPowerRegisterDescriptor(LuxPowerRegisterId::Pv2Power, descriptor);

        case LuxPowerMetric::BatterySoc:
            return luxPowerRegisterDescriptor(LuxPowerRegisterId::BatterySoc, descriptor);

        case LuxPowerMetric::BatteryChargePower:
            return luxPowerRegisterDescriptor(LuxPowerRegisterId::BatteryChargePower, descriptor);

        case LuxPowerMetric::BatteryDischargePower:
            return luxPowerRegisterDescriptor(LuxPowerRegisterId::BatteryDischargePower, descriptor);
    }

    return false;
}

LuxPowerMetricValue::Type metricValueTypeFromDescriptor(const LuxPowerRegisterDescriptor &descriptor)
{
    switch (descriptor.type)
    {
        case LuxPowerRegisterValueType::Bit:
            return LuxPowerMetricValue::Type::Boolean;

        case LuxPowerRegisterValueType::U16:
        case LuxPowerRegisterValueType::U32:
            return LuxPowerMetricValue::Type::Float;
    }

    return LuxPowerMetricValue::Type::Float;
}
constexpr uint8_t PROTOCOL_VERSION = 0x01;
constexpr uint8_t ACTION_VALUE = 0x00;
constexpr uint8_t AUX_VALUE = 0xC2;
constexpr uint16_t PACKET_SIZE = 38;
constexpr uint8_t HEADER_A1 = 0xA1;
constexpr uint8_t HEADER_1A = 0x1A;

}  // namespace

bool luxPowerRegisterDescriptor(LuxPowerRegisterId id, LuxPowerRegisterDescriptor &descriptor)
{
    switch (id)
    {
        case LuxPowerRegisterId::Pv1EnergyTotal:
            descriptor = kVerifiedRegisters[0];
            return true;

        case LuxPowerRegisterId::FeedInGridEnabled:
            descriptor = kVerifiedRegisters[1];
            return true;

        case LuxPowerRegisterId::Pv1Power:
            descriptor = kVerifiedRegisters[2];
            return true;

        case LuxPowerRegisterId::Pv2Power:
            descriptor = kVerifiedRegisters[3];
            return true;

        case LuxPowerRegisterId::BatterySoc:
            descriptor = kVerifiedRegisters[4];
            return true;

        case LuxPowerRegisterId::BatteryChargePower:
            descriptor = kVerifiedRegisters[5];
            return true;

        case LuxPowerRegisterId::BatteryDischargePower:
            descriptor = kVerifiedRegisters[6];
            return true;
    }

    return false;
}

LuxPowerTCP::LuxPowerTCP(LuxPowerTransport &transport)
    : transport_(transport),
#ifdef ARDUINO
      ownedArduinoTransport_(nullptr),
#endif
      port_(0),
      dongleSerial_(nullptr),
      inverterSerial_(nullptr),
      lastError_("No error"),
      lastU16_(0),
      lastU32_(0),
      cache_(),
      inputPollInterval_(1000),
      inputBlock2PollInterval_(1000),
      holdingPollInterval_(30000),
      lastConnectAttemptMs_(0),
      consecutiveResponseTimeouts_(0)
{
    host_[0] = '\0';
    cache_.clear();
}

#ifdef ARDUINO
LuxPowerTCP::LuxPowerTCP(Client &client)
    : LuxPowerTCP(*new LuxPowerArduinoTransport(client))
{
    ownedArduinoTransport_ = static_cast<LuxPowerArduinoTransport *>(&transport_);
}

LuxPowerTCP::~LuxPowerTCP()
{
    delete ownedArduinoTransport_;
}

void LuxPowerTCP::begin(IPAddress ip, uint16_t port, const char *dongleSerial, const char *inverterSerial)
{
    const String host = ip.toString();
    begin(host.c_str(), port, dongleSerial, inverterSerial);
}
#endif

void LuxPowerTCP::begin(const char *host, uint16_t port, const char *dongleSerial, const char *inverterSerial)
{
    strncpy(host_, host == nullptr ? "" : host, sizeof(host_) - 1);
    host_[sizeof(host_) - 1] = '\0';
    port_ = port;
    dongleSerial_ = dongleSerial;
    inverterSerial_ = inverterSerial;
    lastError_ = "No error";
    consecutiveResponseTimeouts_ = 0;
}

uint16_t LuxPowerTCP::lastU16() const
{
    return lastU16_;
}

uint32_t LuxPowerTCP::lastU32() const
{
    return lastU32_;
}

const char *LuxPowerTCP::lastError() const
{
    return lastError_;
}

bool LuxPowerTCP::inputCacheValid() const
{
    return cache_.inputValid;
}

uint32_t LuxPowerTCP::inputCacheAge() const
{
    if (!cache_.inputValid)
    {
        return 0;
    }

    return luxPowerMillis() - cache_.inputUpdated;
}

bool LuxPowerTCP::pv1EnergyTotal(float &kWh) const
{
    if (!cache_.inputValid)
    {
        kWh = 0.0f;
        return false;
    }

    LuxPowerRegisterDescriptor descriptor{};
    if (!luxPowerRegisterDescriptor(LuxPowerRegisterId::Pv1EnergyTotal, descriptor))
    {
        kWh = 0.0f;
        return false;
    }

    if (descriptor.space != LuxPowerRegisterSpace::Input || descriptor.type != LuxPowerRegisterValueType::U32)
    {
        kWh = 0.0f;
        return false;
    }

    uint32_t rawValue = 0;
    if (!cachedInputU32(descriptor.address, rawValue))
    {
        kWh = 0.0f;
        return false;
    }

    kWh = static_cast<float>(rawValue) * descriptor.scale;
    return true;
}

bool LuxPowerTCP::pv1Power(float &watts) const
{
    if (!cache_.inputValid)
    {
        watts = 0.0f;
        return false;
    }

    LuxPowerRegisterDescriptor descriptor{};
    if (!luxPowerRegisterDescriptor(LuxPowerRegisterId::Pv1Power, descriptor))
    {
        watts = 0.0f;
        return false;
    }

    if (descriptor.space != LuxPowerRegisterSpace::Input || descriptor.type != LuxPowerRegisterValueType::U16)
    {
        watts = 0.0f;
        return false;
    }

    uint16_t rawValue = 0;
    if (!cachedInputU16(descriptor.address, rawValue))
    {
        watts = 0.0f;
        return false;
    }

    watts = static_cast<float>(rawValue) * descriptor.scale;
    return true;
}

bool LuxPowerTCP::pv2Power(float &watts) const
{
    if (!cache_.inputValid)
    {
        watts = 0.0f;
        return false;
    }

    LuxPowerRegisterDescriptor descriptor{};
    if (!luxPowerRegisterDescriptor(LuxPowerRegisterId::Pv2Power, descriptor))
    {
        watts = 0.0f;
        return false;
    }

    if (descriptor.space != LuxPowerRegisterSpace::Input || descriptor.type != LuxPowerRegisterValueType::U16)
    {
        watts = 0.0f;
        return false;
    }

    uint16_t rawValue = 0;
    if (!cachedInputU16(descriptor.address, rawValue))
    {
        watts = 0.0f;
        return false;
    }

    watts = static_cast<float>(rawValue) * descriptor.scale;
    return true;
}

bool LuxPowerTCP::batterySoc(float &percent) const
{
    if (!cache_.inputValid)
    {
        percent = 0.0f;
        return false;
    }

    LuxPowerRegisterDescriptor descriptor{};
    if (!luxPowerRegisterDescriptor(LuxPowerRegisterId::BatterySoc, descriptor))
    {
        percent = 0.0f;
        return false;
    }

    if (descriptor.space != LuxPowerRegisterSpace::Input || descriptor.type != LuxPowerRegisterValueType::U16)
    {
        percent = 0.0f;
        return false;
    }

    uint16_t rawValue = 0;
    if (!cachedInputU16(descriptor.address, rawValue))
    {
        percent = 0.0f;
        return false;
    }

    percent = static_cast<float>(rawValue & 0xFFU) * descriptor.scale;
    return true;
}

bool LuxPowerTCP::batteryChargePower(float &watts) const
{
    if (!cache_.inputValid)
    {
        watts = 0.0f;
        return false;
    }

    LuxPowerRegisterDescriptor descriptor{};
    if (!luxPowerRegisterDescriptor(LuxPowerRegisterId::BatteryChargePower, descriptor))
    {
        watts = 0.0f;
        return false;
    }

    if (descriptor.space != LuxPowerRegisterSpace::Input || descriptor.type != LuxPowerRegisterValueType::U16)
    {
        watts = 0.0f;
        return false;
    }

    uint16_t rawValue = 0;
    if (!cachedInputU16(descriptor.address, rawValue))
    {
        watts = 0.0f;
        return false;
    }

    watts = static_cast<float>(rawValue) * descriptor.scale;
    return true;
}

bool LuxPowerTCP::batteryDischargePower(float &watts) const
{
    if (!cache_.inputValid)
    {
        watts = 0.0f;
        return false;
    }

    LuxPowerRegisterDescriptor descriptor{};
    if (!luxPowerRegisterDescriptor(LuxPowerRegisterId::BatteryDischargePower, descriptor))
    {
        watts = 0.0f;
        return false;
    }

    if (descriptor.space != LuxPowerRegisterSpace::Input || descriptor.type != LuxPowerRegisterValueType::U16)
    {
        watts = 0.0f;
        return false;
    }

    uint16_t rawValue = 0;
    if (!cachedInputU16(descriptor.address, rawValue))
    {
        watts = 0.0f;
        return false;
    }

    watts = static_cast<float>(rawValue) * descriptor.scale;
    return true;
}

bool LuxPowerTCP::metricValue(LuxPowerMetric metric, LuxPowerMetricValue &value) const
{
    switch (metric)
    {
        case LuxPowerMetric::Pv1EnergyTotal:
        {
            float kWh = 0.0f;
            const bool ok = pv1EnergyTotal(kWh);
            if (!ok)
            {
                return false;
            }
            value.type = LuxPowerMetricValue::Type::Float;
            value.floatValue = kWh;
            value.boolValue = false;
            return true;
        }

        case LuxPowerMetric::FeedInGridEnabled:
        {
            bool enabled = false;
            const bool ok = feedInGridEnabled(enabled);
            if (!ok)
            {
                return false;
            }
            value.type = LuxPowerMetricValue::Type::Boolean;
            value.floatValue = 0.0f;
            value.boolValue = enabled;
            return true;
        }

        case LuxPowerMetric::Pv1Power:
        {
            float watts = 0.0f;
            const bool ok = pv1Power(watts);
            if (!ok)
            {
                return false;
            }
            value.type = LuxPowerMetricValue::Type::Float;
            value.floatValue = watts;
            value.boolValue = false;
            return true;
        }

        case LuxPowerMetric::Pv2Power:
        {
            float watts = 0.0f;
            const bool ok = pv2Power(watts);
            if (!ok)
            {
                return false;
            }
            value.type = LuxPowerMetricValue::Type::Float;
            value.floatValue = watts;
            value.boolValue = false;
            return true;
        }

        case LuxPowerMetric::BatterySoc:
        {
            float percent = 0.0f;
            const bool ok = batterySoc(percent);
            if (!ok)
            {
                return false;
            }
            value.type = LuxPowerMetricValue::Type::Float;
            value.floatValue = percent;
            value.boolValue = false;
            return true;
        }

        case LuxPowerMetric::BatteryChargePower:
        {
            float watts = 0.0f;
            const bool ok = batteryChargePower(watts);
            if (!ok)
            {
                return false;
            }
            value.type = LuxPowerMetricValue::Type::Float;
            value.floatValue = watts;
            value.boolValue = false;
            return true;
        }

        case LuxPowerMetric::BatteryDischargePower:
        {
            float watts = 0.0f;
            const bool ok = batteryDischargePower(watts);
            if (!ok)
            {
                return false;
            }
            value.type = LuxPowerMetricValue::Type::Float;
            value.floatValue = watts;
            value.boolValue = false;
            return true;
        }
    }

    return false;
}

bool LuxPowerTCP::metricDescriptor(LuxPowerMetric metric, LuxPowerMetricDescriptor &descriptor) const
{
    LuxPowerRegisterDescriptor registerDescriptor{};
    if (!lookupMetricDescriptor(metric, registerDescriptor))
    {
        return false;
    }

    descriptor.id = metric;
    descriptor.name = registerDescriptor.name;
    descriptor.unit = registerDescriptor.unit;
    descriptor.type = metricValueTypeFromDescriptor(registerDescriptor);
    return true;
}

bool LuxPowerTCP::feedInGridEnabled(bool &enabled) const
{
    LuxPowerRegisterDescriptor descriptor{};
    if (!luxPowerRegisterDescriptor(LuxPowerRegisterId::FeedInGridEnabled, descriptor))
    {
        enabled = false;
        return false;
    }

    if (descriptor.space != LuxPowerRegisterSpace::Holding || descriptor.type != LuxPowerRegisterValueType::Bit)
    {
        enabled = false;
        return false;
    }

    if (descriptor.space == LuxPowerRegisterSpace::Holding && !cache_.holdingValid)
    {
        enabled = false;
        return false;
    }

    uint16_t rawValue = 0;
    if (descriptor.space == LuxPowerRegisterSpace::Holding)
    {
        if (!cachedHoldingU16(descriptor.address, rawValue))
        {
            enabled = false;
            return false;
        }
    }
    else
    {
        if (!cachedInputU16(descriptor.address, rawValue))
        {
            enabled = false;
            return false;
        }
    }

    const uint16_t mask = static_cast<uint16_t>(1U << descriptor.bit);
    enabled = (rawValue & mask) != 0;
    return true;
}

bool LuxPowerTCP::holdingCacheValid() const
{
    return cache_.holdingValid;
}

uint32_t LuxPowerTCP::holdingCacheAge() const
{
    if (!cache_.holdingValid)
    {
        return 0;
    }

    return luxPowerMillis() - cache_.holdingUpdated;
}

bool LuxPowerTCP::cachedInputU16(uint16_t address, uint16_t &value) const
{
    if (address < kInputBlock1Size)
    {
        if (!cache_.inputValid)
        {
            return false;
        }
    }
    else if (address < kInputTotalSize)
    {
        if (!cache_.inputBlock2Valid)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    value = cache_.input[address];
    return true;
}

bool LuxPowerTCP::cachedInputU32(uint16_t address, uint32_t &value) const
{
    // Both words of a U32 pair must fall inside the SAME poll block; a pair
    // spanning the block1/block2 boundary (address == kInputBlock1Size - 1)
    // is rejected rather than mixing independently-timed cache blocks.
    if (address < kInputBlock1Size - 1)
    {
        if (!cache_.inputValid)
        {
            return false;
        }
    }
    else if (address >= kInputBlock1Size && address < kInputTotalSize - 1)
    {
        if (!cache_.inputBlock2Valid)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    const uint16_t lowWord = cache_.input[address];
    const uint16_t highWord = cache_.input[address + 1];
    value = static_cast<uint32_t>(lowWord) | (static_cast<uint32_t>(highWord) << 16);
    return true;
}

bool LuxPowerTCP::inputBlock2CacheValid() const
{
    return cache_.inputBlock2Valid;
}

uint32_t LuxPowerTCP::inputBlock2CacheAge() const
{
    if (!cache_.inputBlock2Valid)
    {
        return 0;
    }

    return luxPowerMillis() - cache_.inputBlock2Updated;
}

bool LuxPowerTCP::cachedHoldingU16(uint16_t address, uint16_t &value) const
{
    if (!cache_.holdingValid || address >= 125)
    {
        return false;
    }

    value = cache_.holding[address];
    return true;
}

bool LuxPowerTCP::cachedHoldingU32(uint16_t address, uint32_t &value) const
{
    if (!cache_.holdingValid || address >= 124)
    {
        return false;
    }

    const uint16_t lowWord = cache_.holding[address];
    const uint16_t highWord = cache_.holding[address + 1];
    value = static_cast<uint32_t>(lowWord) | (static_cast<uint32_t>(highWord) << 16);
    return true;
}

#ifdef ARDUINO
void LuxPowerTCP::dumpInputRegisters(Stream &out) const
{
    if (!cache_.inputValid)
    {
        out.println("Input cache invalid.");
        return;
    }

    for (uint16_t index = 0; index < kInputBlock1Size; ++index)
    {
        out.printf("INPUT[%03u] = %u\n",
                   static_cast<unsigned int>(index),
                   static_cast<unsigned int>(cache_.input[index]));
    }

    if (!cache_.inputBlock2Valid)
    {
        out.println("Input block 2 (125..249) cache invalid.");
        return;
    }

    for (uint16_t index = kInputBlock1Size; index < kInputTotalSize; ++index)
    {
        out.printf("INPUT[%03u] = %u\n",
                   static_cast<unsigned int>(index),
                   static_cast<unsigned int>(cache_.input[index]));
    }
}

void LuxPowerTCP::dumpHoldingRegisters(Stream &out) const
{
    if (!cache_.holdingValid)
    {
        out.println("Holding cache invalid.");
        return;
    }

    for (uint16_t index = 0; index < 125; ++index)
    {
        out.printf("HOLDING[%03u] = %u\n",
                   static_cast<unsigned int>(index),
                   static_cast<unsigned int>(cache_.holding[index]));
    }
}

bool LuxPowerTCP::dumpChangedInputRegisters(Stream &out, uint16_t minimumDelta)
{
    if (!cache_.inputValid)
    {
        out.println("Input cache invalid.");
        return false;
    }

    if (!previousInputValid_)
    {
        for (uint16_t index = 0; index < 125; ++index)
        {
            previousInput_[index] = cache_.input[index];
        }

        previousInputValid_ = true;
        out.println("Input change baseline captured.");
        return false;
    }

    bool printedAny = false;

    for (uint16_t index = 0; index < 125; ++index)
    {
        const int32_t previous = static_cast<int32_t>(previousInput_[index]);
        const int32_t current = static_cast<int32_t>(cache_.input[index]);
        const int32_t delta = current - previous;
        const int32_t absoluteDelta = delta < 0 ? -delta : delta;

        if (absoluteDelta >= static_cast<int32_t>(minimumDelta))
        {
            out.printf("INPUT[%03u]: %u -> %u (delta %+ld)\n",
                       static_cast<unsigned int>(index),
                       static_cast<unsigned int>(previousInput_[index]),
                       static_cast<unsigned int>(cache_.input[index]),
                       static_cast<long>(delta));
            printedAny = true;
        }
    }

    for (uint16_t index = 0; index < 125; ++index)
    {
        previousInput_[index] = cache_.input[index];
    }

    return printedAny;
}

bool LuxPowerTCP::dumpChangedHoldingRegisters(Stream &out, uint16_t minimumDelta)
{
    if (!cache_.holdingValid)
    {
        out.println("Holding cache invalid.");
        return false;
    }

    if (!previousHoldingValid_)
    {
        for (uint16_t index = 0; index < 125; ++index)
        {
            previousHolding_[index] = cache_.holding[index];
        }

        previousHoldingValid_ = true;
        out.println("Holding change baseline captured.");
        return false;
    }

    bool printedAny = false;

    for (uint16_t index = 0; index < 125; ++index)
    {
        const int32_t previous = static_cast<int32_t>(previousHolding_[index]);
        const int32_t current = static_cast<int32_t>(cache_.holding[index]);
        const int32_t delta = current - previous;
        const int32_t absoluteDelta = delta < 0 ? -delta : delta;

        if (absoluteDelta >= static_cast<int32_t>(minimumDelta))
        {
            out.printf("HOLDING[%03u]: %u -> %u (delta %+ld)\n",
                       static_cast<unsigned int>(index),
                       static_cast<unsigned int>(previousHolding_[index]),
                       static_cast<unsigned int>(cache_.holding[index]),
                       static_cast<long>(delta));
            printedAny = true;
        }
    }

    for (uint16_t index = 0; index < 125; ++index)
    {
        previousHolding_[index] = cache_.holding[index];
    }

    return printedAny;
}
#endif

bool LuxPowerTCP::connect()
{
    if (transport_.connected())
    {
        lastError_ = "No error";
        return true;
    }

    if (lastConnectAttemptMs_ != 0 && (luxPowerMillis() - lastConnectAttemptMs_) < 1000)
    {
        lastError_ = "Reconnect throttled";
        return false;
    }

    lastConnectAttemptMs_ = luxPowerMillis();
    luxPowerLogf("Подключение к LuxPower %s:%u...\n", host_, port_);

    if (!transport_.connect(host_, port_))
    {
        lastError_ = "TCP connection failed";
        luxPowerLog("TCP-подключение не удалось.");
        return false;
    }

    luxPowerLog("TCP подключен.");
    consecutiveResponseTimeouts_ = 0;
    lastError_ = "No error";
    return true;
}

void LuxPowerTCP::disconnect()
{
    if (transport_.connected())
    {
        transport_.disconnect();
    }

    consecutiveResponseTimeouts_ = 0;
}

bool LuxPowerTCP::connected() const
{
    return transport_.connected();
}

bool LuxPowerTCP::discardInitialDongleData()
{
    const uint32_t startedAt = luxPowerMillis();
    size_t discarded = 0;

    while (luxPowerMillis() - startedAt < 1000)
    {
        while (transport_.available())
        {
            transport_.read();
            discarded++;
        }

        luxPowerDelay(10);
    }

    if (discarded > 0)
    {
        luxPowerLogf("Отброшено стартовых данных: %u байт\n", static_cast<unsigned int>(discarded));
    }

    return true;
}

bool LuxPowerTCP::consumeUnsolicitedFrames(uint32_t timeoutMs)
{
    const uint32_t startedAt = luxPowerMillis();
    bool consumedAny = false;

    while (luxPowerMillis() - startedAt < timeoutMs)
    {
        if (!transport_.available())
        {
            break;
        }

        uint8_t frame[1024];
        size_t frameLength = 0;
        if (!readLuxFrame(frame, sizeof(frame), frameLength, 20))
        {
            break;
        }

        consumedAny = true;
        luxPowerLogf("Пропущен unsolicited frame: %u байт\n", static_cast<unsigned int>(frameLength));
    }

    return consumedAny;
}

uint16_t LuxPowerTCP::modbusCrc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;

    for (size_t pos = 0; pos < length; pos++)
    {
        crc ^= data[pos];

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

size_t LuxPowerTCP::buildReadRequest(uint8_t *packet, size_t packetCapacity, uint16_t startRegister, uint16_t registerCount, uint8_t functionCode)
{
    if (packetCapacity < PACKET_SIZE)
    {
        return 0;
    }

    size_t pos = 0;

    packet[pos++] = HEADER_A1;
    packet[pos++] = HEADER_1A;
    packet[pos++] = PROTOCOL_VERSION;
    packet[pos++] = 0x00;
    packet[pos++] = 0x20;
    packet[pos++] = 0x00;
    packet[pos++] = 0x01;
    packet[pos++] = AUX_VALUE;

    if (dongleSerial_ != nullptr)
    {
        memcpy(&packet[pos], dongleSerial_, 10);
    }
    else
    {
        memset(&packet[pos], 0, 10);
    }
    pos += 10;

    packet[pos++] = 0x12;
    packet[pos++] = 0x00;

    packet[pos++] = ACTION_VALUE;
    packet[pos++] = functionCode;

    if (inverterSerial_ != nullptr)
    {
        memcpy(&packet[pos], inverterSerial_, 10);
    }
    else
    {
        memset(&packet[pos], 0, 10);
    }
    pos += 10;

    packet[pos++] = startRegister & 0xFF;
    packet[pos++] = (startRegister >> 8) & 0xFF;
    packet[pos++] = registerCount & 0xFF;
    packet[pos++] = (registerCount >> 8) & 0xFF;

    const uint16_t crc = modbusCrc16(&packet[20], 16);
    packet[pos++] = crc & 0xFF;
    packet[pos++] = (crc >> 8) & 0xFF;

    return pos;
}

bool LuxPowerTCP::readLuxFrame(uint8_t *buffer, size_t capacity, size_t &frameLength, uint32_t timeoutMs)
{
    frameLength = 0;
    uint32_t startedAt = luxPowerMillis();

    bool foundA1 = false;

    while (luxPowerMillis() - startedAt < timeoutMs)
    {
        while (transport_.available())
        {
            uint8_t value = transport_.read();

            if (!foundA1)
            {
                if (value == HEADER_A1)
                {
                    buffer[0] = value;
                    foundA1 = true;
                }

                continue;
            }

            if (value == HEADER_1A)
            {
                buffer[1] = value;
                frameLength = 2;
                break;
            }

            foundA1 = (value == HEADER_A1);

            if (foundA1)
            {
                buffer[0] = HEADER_A1;
            }
        }

        if (frameLength == 2)
        {
            break;
        }

        luxPowerDelay(2);
    }

    if (frameLength != 2)
    {
        return false;
    }

    while (frameLength < 6 && luxPowerMillis() - startedAt < timeoutMs)
    {
        while (transport_.available() && frameLength < 6)
        {
            buffer[frameLength++] = transport_.read();
        }

        luxPowerDelay(2);
    }

    if (frameLength < 6)
    {
        luxPowerLog("Не удалось получить полный заголовок.");
        return false;
    }

    const uint16_t payloadLength =
        buffer[4] |
        (static_cast<uint16_t>(buffer[5]) << 8);

    const size_t totalLength = payloadLength + 6;

    if (totalLength > capacity)
    {
        luxPowerLogf("Пакет слишком большой: %u байт, буфер %u байт\n",
                      static_cast<unsigned int>(totalLength),
                      static_cast<unsigned int>(capacity));
        return false;
    }

    while (frameLength < totalLength && luxPowerMillis() - startedAt < timeoutMs)
    {
        while (transport_.available() && frameLength < totalLength)
        {
            buffer[frameLength++] = transport_.read();
        }

        luxPowerDelay(2);
    }

    if (frameLength != totalLength)
    {
        luxPowerLogf("Неполный пакет: %u из %u байт\n",
                      static_cast<unsigned int>(frameLength),
                      static_cast<unsigned int>(totalLength));
        return false;
    }

    return true;
}

bool LuxPowerTCP::parseRegisterResponse(const uint8_t *packet, size_t length, uint16_t expectedRegister, uint8_t expectedFunction, uint16_t &value16)
{
    if (length < 39)
    {
        lastError_ = "U16 response too short";
        luxPowerLogf("Ответ слишком короткий: %u байт\n",
                      static_cast<unsigned int>(length));
        return false;
    }

    if (packet[0] != HEADER_A1 || packet[1] != HEADER_1A)
    {
        lastError_ = "U16 response header invalid";
        luxPowerLog("Нет заголовка A1 1A.");
        return false;
    }

    const uint16_t frameLength =
        packet[4] |
        (static_cast<uint16_t>(packet[5]) << 8);

    const size_t expectedLength = frameLength + 6;

    if (length < expectedLength)
    {
        lastError_ = "U16 response incomplete";
        luxPowerLogf("Неполный пакет: %u из %u байт\n",
                      static_cast<unsigned int>(length),
                      static_cast<unsigned int>(expectedLength));
        return false;
    }

    const uint16_t receivedCrc =
        packet[expectedLength - 2] |
        (static_cast<uint16_t>(packet[expectedLength - 1]) << 8);

    const uint16_t calculatedCrc = modbusCrc16(&packet[20], expectedLength - 22);

    luxPowerLogf("CRC: получен 0x%04X, рассчитан 0x%04X\n", receivedCrc, calculatedCrc);

    if (receivedCrc != calculatedCrc)
    {
        lastError_ = "U16 response CRC mismatch";
        luxPowerLog("Ошибка CRC.");
        return false;
    }

    const uint8_t functionCode = packet[21];

    if (functionCode != expectedFunction)
    {
        lastError_ = "U16 response function mismatch";
        luxPowerLogf("Неожиданная функция: 0x%02X\n", functionCode);
        return false;
    }

    const uint16_t returnedRegister =
        packet[32] |
        (static_cast<uint16_t>(packet[33]) << 8);

    if (returnedRegister != expectedRegister)
    {
        lastError_ = "U16 response register mismatch";
        luxPowerLogf("Получен регистр %u вместо %u\n", returnedRegister, expectedRegister);
        return false;
    }

    const uint8_t byteCount = packet[34];

    if (byteCount < 2)
    {
        lastError_ = "U16 response data too short";
        luxPowerLogf("Недостаточно данных: %u байт\n", byteCount);
        return false;
    }

    value16 = packet[35] | (static_cast<uint16_t>(packet[36]) << 8);
    lastError_ = "No error";
    return true;
}

bool LuxPowerTCP::parseRegisterResponseU32(const uint8_t *packet, size_t length, uint16_t expectedRegister, uint8_t expectedFunction, uint32_t &value32)
{
    if (length < 41)
    {
        lastError_ = "U32 response too short";
        luxPowerLogf("Ответ слишком короткий: %u байт\n",
                      static_cast<unsigned int>(length));
        return false;
    }

    if (packet[0] != HEADER_A1 || packet[1] != HEADER_1A)
    {
        lastError_ = "U32 response header invalid";
        luxPowerLog("Нет заголовка A1 1A.");
        return false;
    }

    const uint16_t frameLength =
        packet[4] |
        (static_cast<uint16_t>(packet[5]) << 8);

    const size_t expectedLength = frameLength + 6;

    if (length < expectedLength)
    {
        lastError_ = "U32 response incomplete";
        luxPowerLogf("Неполный пакет: %u из %u байт\n",
                      static_cast<unsigned int>(length),
                      static_cast<unsigned int>(expectedLength));
        return false;
    }

    const uint16_t receivedCrc =
        packet[expectedLength - 2] |
        (static_cast<uint16_t>(packet[expectedLength - 1]) << 8);

    const uint16_t calculatedCrc = modbusCrc16(&packet[20], expectedLength - 22);

    luxPowerLogf("CRC: получен 0x%04X, рассчитан 0x%04X\n", receivedCrc, calculatedCrc);

    if (receivedCrc != calculatedCrc)
    {
        lastError_ = "U32 response CRC mismatch";
        luxPowerLog("Ошибка CRC.");
        return false;
    }

    const uint8_t functionCode = packet[21];

    if (functionCode != expectedFunction)
    {
        lastError_ = "U32 response function mismatch";
        luxPowerLogf("Неожиданная функция: 0x%02X\n", functionCode);
        return false;
    }

    const uint16_t returnedRegister =
        packet[32] |
        (static_cast<uint16_t>(packet[33]) << 8);

    if (returnedRegister != expectedRegister)
    {
        lastError_ = "U32 response register mismatch";
        luxPowerLogf("Получен регистр %u вместо %u\n", returnedRegister, expectedRegister);
        return false;
    }

    const uint8_t byteCount = packet[34];

    if (byteCount < 4)
    {
        lastError_ = "U32 response data too short";
        luxPowerLogf("Недостаточно данных: %u байт\n", byteCount);
        return false;
    }

    const uint16_t lowWord = packet[35] | (static_cast<uint16_t>(packet[36]) << 8);
    const uint16_t highWord = packet[37] | (static_cast<uint16_t>(packet[38]) << 8);

    value32 = static_cast<uint32_t>(lowWord) | (static_cast<uint32_t>(highWord) << 16);
    lastError_ = "No error";
    return true;
}

bool LuxPowerTCP::readRegister(uint16_t address, uint8_t functionCode, uint16_t &value16)
{
    if (!connect())
    {
        return false;
    }

    consumeUnsolicitedFrames(50);

    uint8_t request[38];
    const size_t requestLength = buildReadRequest(request, sizeof(request), address, 1, functionCode);

    if (requestLength == 0)
    {
        luxPowerLog("Ошибка формирования запроса.");
        disconnect();
        return false;
    }

    luxPowerLog("Запрос HEX: ");
    luxPowerLogHex(request, requestLength);

    const size_t written = transport_.write(request, requestLength);
    transport_.flush();

    if (written != requestLength)
    {
        luxPowerLogf("Отправлено только %u из %u байт.\n",
                      static_cast<unsigned int>(written),
                      static_cast<unsigned int>(requestLength));
        disconnect();
        return false;
    }

    uint8_t response[1024];
    size_t responseLength = 0;

    const uint32_t overallStartedAt = luxPowerMillis();

    while (luxPowerMillis() - overallStartedAt < MATCHING_RESPONSE_TIMEOUT_MS)
    {
        if (!readLuxFrame(response, sizeof(response), responseLength, MATCHING_RESPONSE_TIMEOUT_MS))
        {
            continue;
        }

        luxPowerLogf("Получен полный пакет: %u байт\n", static_cast<unsigned int>(responseLength));

        if (responseLength < 36)
        {
            luxPowerLog("Пакет слишком короткий, пропускаем.");
            continue;
        }

        const uint8_t protocol = response[2];
        const uint8_t action = response[20];
        const uint8_t responseFunctionCode = response[21];
        const uint16_t returnedRegister = response[32] | (static_cast<uint16_t>(response[33]) << 8);

        luxPowerLogf("Пакет: protocol=%u, action=%u, function=0x%02X, register=%u\n",
                      protocol, action, responseFunctionCode, returnedRegister);

        if (responseFunctionCode != functionCode || returnedRegister != address)
        {
            luxPowerLog("Это потоковая телеметрия или другой ответ — пропускаем.");
            continue;
        }

        luxPowerLog("Нужный ответ HEX: ");
        luxPowerLogHex(response, responseLength);

        const bool result = parseRegisterResponse(response, responseLength, address, functionCode, value16);
        if (result)
        {
            lastU16_ = value16;
            consecutiveResponseTimeouts_ = 0;
            lastError_ = "No error";
        }
        else
        {
            disconnect();
        }
        return result;
    }

    if (!transport_.connected())
    {
        lastError_ = "Socket disconnected during register read";
        luxPowerLog("Сокет отключился во время ожидания ответа.");
        disconnect();
        return false;
    }

    consecutiveResponseTimeouts_ += 1;
    if (consecutiveResponseTimeouts_ >= 3)
    {
        lastError_ = "3 consecutive response timeouts";
        luxPowerLogf("Таймаут ответа #%u/3; отключаю сокет и требую переподключения.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
        disconnect();
    }
    else
    {
        lastError_ = "Response timeout";
        luxPowerLogf("Таймаут ответа #%u/3; держу сокет открытым.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
    }

    return false;
}

bool LuxPowerTCP::readRegisterU32(uint16_t address, uint8_t functionCode, uint32_t &value32)
{
    if (!connect())
    {
        return false;
    }

    consumeUnsolicitedFrames(50);

    uint8_t request[38];
    const size_t requestLength = buildReadRequest(request, sizeof(request), address, 2, functionCode);

    if (requestLength == 0)
    {
        luxPowerLog("Ошибка формирования запроса.");
        disconnect();
        return false;
    }

    luxPowerLog("Запрос HEX: ");
    luxPowerLogHex(request, requestLength);

    const size_t written = transport_.write(request, requestLength);
    transport_.flush();

    if (written != requestLength)
    {
        luxPowerLogf("Отправлено только %u из %u байт.\n",
                      static_cast<unsigned int>(written),
                      static_cast<unsigned int>(requestLength));
        disconnect();
        return false;
    }

    uint8_t response[1024];
    size_t responseLength = 0;

    const uint32_t overallStartedAt = luxPowerMillis();

    while (luxPowerMillis() - overallStartedAt < MATCHING_RESPONSE_TIMEOUT_MS)
    {
        if (!readLuxFrame(response, sizeof(response), responseLength, MATCHING_RESPONSE_TIMEOUT_MS))
        {
            continue;
        }

        luxPowerLogf("Получен полный пакет: %u байт\n", static_cast<unsigned int>(responseLength));

        if (responseLength < 36)
        {
            luxPowerLog("Пакет слишком короткий, пропускаем.");
            continue;
        }

        const uint8_t protocol = response[2];
        const uint8_t action = response[20];
        const uint8_t responseFunctionCode = response[21];
        const uint16_t returnedRegister = response[32] | (static_cast<uint16_t>(response[33]) << 8);

        luxPowerLogf("Пакет: protocol=%u, action=%u, function=0x%02X, register=%u\n",
                      protocol, action, responseFunctionCode, returnedRegister);

        if (responseFunctionCode != functionCode || returnedRegister != address)
        {
            luxPowerLog("Это потоковая телеметрия или другой ответ — пропускаем.");
            continue;
        }

        luxPowerLog("Нужный ответ HEX: ");
        luxPowerLogHex(response, responseLength);

        const bool result = parseRegisterResponseU32(response, responseLength, address, functionCode, value32);
        if (result)
        {
            lastU32_ = value32;
            consecutiveResponseTimeouts_ = 0;
            lastError_ = "No error";
        }
        else
        {
            disconnect();
        }
        return result;
    }

    if (!transport_.connected())
    {
        lastError_ = "Socket disconnected during register read";
        luxPowerLog("Сокет отключился во время ожидания ответа.");
        disconnect();
        return false;
    }

    consecutiveResponseTimeouts_ += 1;
    if (consecutiveResponseTimeouts_ >= 3)
    {
        lastError_ = "3 consecutive response timeouts";
        luxPowerLogf("Таймаут ответа #%u/3; отключаю сокет и требую переподключения.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
        disconnect();
    }
    else
    {
        lastError_ = "Response timeout";
        luxPowerLogf("Таймаут ответа #%u/3; держу сокет открытым.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
    }

    return false;
}

bool LuxPowerTCP::refreshInputBlock()
{
    if (!connect())
    {
        return false;
    }

    consumeUnsolicitedFrames(50);

    uint8_t request[38];
    const size_t requestLength = buildReadRequest(request, sizeof(request), 0, 125, MODBUS_FUNCTION_READ_INPUT);

    if (requestLength == 0)
    {
        luxPowerLog("Ошибка формирования запроса.");
        disconnect();
        return false;
    }

    luxPowerLog("Запрос HEX: ");
    luxPowerLogHex(request, requestLength);

    const size_t written = transport_.write(request, requestLength);
    transport_.flush();

    if (written != requestLength)
    {
        luxPowerLogf("Отправлено только %u из %u байт.\n",
                      static_cast<unsigned int>(written),
                      static_cast<unsigned int>(requestLength));
        disconnect();
        return false;
    }

    uint8_t response[1024];
    size_t responseLength = 0;

    const uint32_t overallStartedAt = luxPowerMillis();

    while (luxPowerMillis() - overallStartedAt < MATCHING_RESPONSE_TIMEOUT_MS)
    {
        if (!readLuxFrame(response, sizeof(response), responseLength, MATCHING_RESPONSE_TIMEOUT_MS))
        {
            continue;
        }

        luxPowerLogf("Получен полный пакет: %u байт\n", static_cast<unsigned int>(responseLength));

        if (responseLength < 36)
        {
            luxPowerLog("Пакет слишком короткий, пропускаем.");
            continue;
        }

        const uint8_t protocol = response[2];
        const uint8_t action = response[20];
        const uint8_t responseFunctionCode = response[21];
        const uint16_t returnedRegister = response[32] | (static_cast<uint16_t>(response[33]) << 8);

        luxPowerLogf("Пакет: protocol=%u, action=%u, function=0x%02X, register=%u\n",
                      protocol, action, responseFunctionCode, returnedRegister);

        if (responseFunctionCode != MODBUS_FUNCTION_READ_INPUT || returnedRegister != 0)
        {
            luxPowerLog("Это потоковая телеметрия или другой ответ — пропускаем.");
            continue;
        }

        luxPowerLog("Нужный ответ HEX: ");
        luxPowerLogHex(response, responseLength);

        const uint16_t frameLength =
            response[4] |
            (static_cast<uint16_t>(response[5]) << 8);

        const size_t expectedLength = frameLength + 6;

        if (responseLength < expectedLength)
        {
            luxPowerLogf("Неполный пакет: %u из %u байт\n",
                          static_cast<unsigned int>(responseLength),
                          static_cast<unsigned int>(expectedLength));
            disconnect();
            return false;
        }

        const uint16_t receivedCrc =
            response[expectedLength - 2] |
            (static_cast<uint16_t>(response[expectedLength - 1]) << 8);

        const uint16_t calculatedCrc = modbusCrc16(&response[20], expectedLength - 22);

        luxPowerLogf("CRC: получен 0x%04X, рассчитан 0x%04X\n", receivedCrc, calculatedCrc);

        if (receivedCrc != calculatedCrc)
        {
            luxPowerLog("Ошибка CRC.");
            disconnect();
            return false;
        }

        const uint8_t byteCount = response[34];

        if (byteCount != 250)
        {
            luxPowerLogf("Недостаточно данных: %u байт\n", byteCount);
            disconnect();
            return false;
        }

        for (size_t index = 0; index < 125; ++index)
        {
            const size_t offset = 35 + (index * 2);
            cache_.input[index] = response[offset] | (static_cast<uint16_t>(response[offset + 1]) << 8);
        }

        cache_.inputValid = true;
        cache_.inputUpdated = luxPowerMillis();
        consecutiveResponseTimeouts_ = 0;
        lastError_ = "No error";
        return true;
    }

    if (!transport_.connected())
    {
        lastError_ = "Socket disconnected during refresh";
        luxPowerLog("Сокет отключился во время ожидания ответа.");
        disconnect();
        return false;
    }

    consecutiveResponseTimeouts_ += 1;
    if (consecutiveResponseTimeouts_ >= 3)
    {
        lastError_ = "3 consecutive response timeouts";
        luxPowerLogf("Таймаут ответа #%u/3; отключаю сокет и требую переподключения.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
        disconnect();
    }
    else
    {
        lastError_ = "Response timeout";
        luxPowerLogf("Таймаут ответа #%u/3; держу сокет открытым.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
    }

    return false;
}

// Stage 31: second contiguous input block (registers 125..249), added
// solely to reach load_power (register 170) for FAST-class telemetry.
// Mirrors refreshInputBlock() exactly except for the start register and
// which half of the cache/validity state it writes.
bool LuxPowerTCP::refreshInputBlock2()
{
    if (!connect())
    {
        return false;
    }

    consumeUnsolicitedFrames(50);

    uint8_t request[38];
    const size_t requestLength = buildReadRequest(request, sizeof(request), kInputBlock1Size, kInputBlock2Size, MODBUS_FUNCTION_READ_INPUT);

    if (requestLength == 0)
    {
        luxPowerLog("Ошибка формирования запроса (input block 2).");
        disconnect();
        return false;
    }

    luxPowerLog("Запрос HEX (input block 2): ");
    luxPowerLogHex(request, requestLength);

    const size_t written = transport_.write(request, requestLength);
    transport_.flush();

    if (written != requestLength)
    {
        luxPowerLogf("Отправлено только %u из %u байт (input block 2).\n",
                      static_cast<unsigned int>(written),
                      static_cast<unsigned int>(requestLength));
        disconnect();
        return false;
    }

    uint8_t response[1024];
    size_t responseLength = 0;

    const uint32_t overallStartedAt = luxPowerMillis();

    while (luxPowerMillis() - overallStartedAt < MATCHING_RESPONSE_TIMEOUT_MS)
    {
        if (!readLuxFrame(response, sizeof(response), responseLength, MATCHING_RESPONSE_TIMEOUT_MS))
        {
            continue;
        }

        luxPowerLogf("Получен полный пакет (input block 2): %u байт\n", static_cast<unsigned int>(responseLength));

        if (responseLength < 36)
        {
            luxPowerLog("Пакет слишком короткий (input block 2), пропускаем.");
            continue;
        }

        const uint8_t protocol = response[2];
        const uint8_t action = response[20];
        const uint8_t responseFunctionCode = response[21];
        const uint16_t returnedRegister = response[32] | (static_cast<uint16_t>(response[33]) << 8);

        luxPowerLogf("Пакет (input block 2): protocol=%u, action=%u, function=0x%02X, register=%u\n",
                      protocol, action, responseFunctionCode, returnedRegister);

        if (responseFunctionCode != MODBUS_FUNCTION_READ_INPUT || returnedRegister != kInputBlock1Size)
        {
            luxPowerLog("Это потоковая телеметрия или другой ответ (input block 2) — пропускаем.");
            continue;
        }

        luxPowerLog("Нужный ответ HEX (input block 2): ");
        luxPowerLogHex(response, responseLength);

        const uint16_t frameLength =
            response[4] |
            (static_cast<uint16_t>(response[5]) << 8);

        const size_t expectedLength = frameLength + 6;

        if (responseLength < expectedLength)
        {
            luxPowerLogf("Неполный пакет (input block 2): %u из %u байт\n",
                          static_cast<unsigned int>(responseLength),
                          static_cast<unsigned int>(expectedLength));
            disconnect();
            return false;
        }

        const uint16_t receivedCrc =
            response[expectedLength - 2] |
            (static_cast<uint16_t>(response[expectedLength - 1]) << 8);

        const uint16_t calculatedCrc = modbusCrc16(&response[20], expectedLength - 22);

        luxPowerLogf("CRC (input block 2): получен 0x%04X, рассчитан 0x%04X\n", receivedCrc, calculatedCrc);

        if (receivedCrc != calculatedCrc)
        {
            luxPowerLog("Ошибка CRC (input block 2).");
            disconnect();
            return false;
        }

        const uint8_t byteCount = response[34];

        if (byteCount != kInputBlock2Size * 2)
        {
            luxPowerLogf("Недостаточно данных (input block 2): %u байт\n", byteCount);
            disconnect();
            return false;
        }

        for (size_t index = 0; index < kInputBlock2Size; ++index)
        {
            const size_t offset = 35 + (index * 2);
            cache_.input[kInputBlock1Size + index] = response[offset] | (static_cast<uint16_t>(response[offset + 1]) << 8);
        }

        cache_.inputBlock2Valid = true;
        cache_.inputBlock2Updated = luxPowerMillis();
        consecutiveResponseTimeouts_ = 0;
        lastError_ = "No error";
        return true;
    }

    if (!transport_.connected())
    {
        lastError_ = "Socket disconnected during refresh (input block 2)";
        luxPowerLog("Сокет отключился во время ожидания ответа (input block 2).");
        disconnect();
        return false;
    }

    consecutiveResponseTimeouts_ += 1;
    if (consecutiveResponseTimeouts_ >= 3)
    {
        lastError_ = "3 consecutive response timeouts";
        luxPowerLogf("Таймаут ответа (input block 2) #%u/3; отключаю сокет и требую переподключения.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
        disconnect();
    }
    else
    {
        lastError_ = "Response timeout";
        luxPowerLogf("Таймаут ответа (input block 2) #%u/3; держу сокет открытым.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
    }

    return false;
}

bool LuxPowerTCP::poll()
{
    const uint32_t now = luxPowerMillis();
    const bool inputDue = !cache_.inputValid || (now - cache_.inputUpdated) >= inputPollInterval_;
    const bool inputBlock2Due =
        !cache_.inputBlock2Valid || (now - cache_.inputBlock2Updated) >= inputBlock2PollInterval_;
    const bool holdingDue = !cache_.holdingValid || (now - cache_.holdingUpdated) >= holdingPollInterval_;

    if (!inputDue && !inputBlock2Due && !holdingDue)
    {
        return false;
    }

    // Priority: block 1 (FAST-class fields) > block 2 (load_power, also
    // FAST) > holding (SLOW-class only). One poll() call issues at most one
    // blocking round trip; the caller invokes poll() every loop iteration,
    // so both input blocks interleave at roughly their own interval instead
    // of block 2 being starved by block 1.
    if (inputDue)
    {
        return refreshInputBlock();
    }

    if (inputBlock2Due)
    {
        return refreshInputBlock2();
    }

    return refreshHoldingBlock();
}

void LuxPowerTCP::setInputPollInterval(uint32_t intervalMs)
{
    inputPollInterval_ = intervalMs;
}

void LuxPowerTCP::setInputBlock2PollInterval(uint32_t intervalMs)
{
    inputBlock2PollInterval_ = intervalMs;
}

void LuxPowerTCP::setHoldingPollInterval(uint32_t intervalMs)
{
    holdingPollInterval_ = intervalMs;
}

bool LuxPowerTCP::readHoldingU16(uint16_t address)
{
    if (!cache_.holdingValid)
    {
        if (!refreshHoldingBlock())
        {
            return false;
        }
    }

    if (address >= 125)
    {
        return false;
    }

    lastU16_ = cache_.holding[address];
    return true;
}

bool LuxPowerTCP::readInputU16(uint16_t address)
{
    if (!cache_.inputValid)
    {
        if (!refreshInputBlock())
        {
            return false;
        }
    }

    if (address >= 125)
    {
        return false;
    }

    lastU16_ = cache_.input[address];
    return true;
}

bool LuxPowerTCP::readInputU32(uint16_t address)
{
    if (!cache_.inputValid)
    {
        if (!refreshInputBlock())
        {
            return false;
        }
    }

    if (address + 1 >= 125)
    {
        return false;
    }

    const uint16_t lowWord = cache_.input[address];
    const uint16_t highWord = cache_.input[address + 1];
    lastU32_ = static_cast<uint32_t>(lowWord) | (static_cast<uint32_t>(highWord) << 16);
    return true;
}

bool LuxPowerTCP::refreshHoldingBlock()
{
    if (!connect())
    {
        return false;
    }

    consumeUnsolicitedFrames(50);

    uint8_t request[38];
    const size_t requestLength = buildReadRequest(request, sizeof(request), 0, 125, MODBUS_FUNCTION_READ_HOLDING);

    if (requestLength == 0)
    {
        luxPowerLog("Ошибка формирования запроса holding block.");
        disconnect();
        return false;
    }

    luxPowerLog("Scheduled holding refresh: start=0 count=125");
    luxPowerLog("Запрос holding block HEX: ");
    luxPowerLogHex(request, requestLength);

    const size_t written = transport_.write(request, requestLength);
    transport_.flush();

    if (written != requestLength)
    {
        luxPowerLogf("Отправлено только %u из %u байт holding block.\n",
                      static_cast<unsigned int>(written),
                      static_cast<unsigned int>(requestLength));
        disconnect();
        return false;
    }

    uint8_t response[1024];
    size_t responseLength = 0;

    const uint32_t overallStartedAt = luxPowerMillis();

    while (luxPowerMillis() - overallStartedAt < MATCHING_RESPONSE_TIMEOUT_MS)
    {
        if (!readLuxFrame(response, sizeof(response), responseLength, MATCHING_RESPONSE_TIMEOUT_MS))
        {
            continue;
        }

        luxPowerLogf("Получен полный пакет holding block: %u байт\n", static_cast<unsigned int>(responseLength));

        if (responseLength < 36)
        {
            luxPowerLog("Пакет holding block слишком короткий, пропускаем.");
            continue;
        }

        const uint8_t protocol = response[2];
        const uint8_t action = response[20];
        const uint8_t responseFunctionCode = response[21];
        const uint16_t returnedRegister = response[32] | (static_cast<uint16_t>(response[33]) << 8);

        luxPowerLogf("Пакет holding block: protocol=%u, action=%u, function=0x%02X, register=%u\n",
                      protocol, action, responseFunctionCode, returnedRegister);

        if (responseFunctionCode != MODBUS_FUNCTION_READ_HOLDING || returnedRegister != 0)
        {
            luxPowerLog("Это потоковая телеметрия или другой ответ holding block — пропускаем.");
            continue;
        }

        luxPowerLog("Нужный ответ holding block HEX: ");
        luxPowerLogHex(response, responseLength);

        const uint16_t frameLength = response[4] | (static_cast<uint16_t>(response[5]) << 8);
        const size_t expectedLength = frameLength + 6;

        if (responseLength < expectedLength)
        {
            luxPowerLogf("Неполный пакет holding block: %u из %u байт\n",
                          static_cast<unsigned int>(responseLength),
                          static_cast<unsigned int>(expectedLength));
            disconnect();
            return false;
        }

        const uint16_t receivedCrc = response[expectedLength - 2] | (static_cast<uint16_t>(response[expectedLength - 1]) << 8);
        const uint16_t calculatedCrc = modbusCrc16(&response[20], expectedLength - 22);

        luxPowerLogf("CRC holding block: получен 0x%04X, рассчитан 0x%04X\n", receivedCrc, calculatedCrc);

        if (receivedCrc != calculatedCrc)
        {
            luxPowerLog("Ошибка CRC holding block.");
            disconnect();
            return false;
        }

        const uint8_t byteCount = response[34];

        if (byteCount != 250)
        {
            luxPowerLogf("Недостаточно данных holding block: %u байт\n", byteCount);
            disconnect();
            return false;
        }

        for (size_t index = 0; index < 125; ++index)
        {
            const size_t offset = 35 + (index * 2);
            cache_.holding[index] = response[offset] | (static_cast<uint16_t>(response[offset + 1]) << 8);
        }

        cache_.holdingValid = true;
        cache_.holdingUpdated = luxPowerMillis();
        consecutiveResponseTimeouts_ = 0;
        lastError_ = "No error";
        luxPowerLog("Holding cache refreshed by scheduled request: start=0 count=125 age=0 ms");
        return true;
    }

    if (!transport_.connected())
    {
        lastError_ = "Socket disconnected during holding refresh";
        luxPowerLog("Сокет отключился во время ожидания ответа holding block.");
        disconnect();
        return false;
    }

    consecutiveResponseTimeouts_ += 1;
    if (consecutiveResponseTimeouts_ >= 3)
    {
        lastError_ = "3 consecutive response timeouts";
        luxPowerLogf("Таймаут ответа holding block #%u/3; отключаю сокет и требую переподключения.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
        disconnect();
    }
    else
    {
        lastError_ = "Response timeout";
        luxPowerLogf("Таймаут ответа holding block #%u/3; держу сокет открытым.\n",
                      static_cast<unsigned int>(consecutiveResponseTimeouts_));
    }

    return false;
}
