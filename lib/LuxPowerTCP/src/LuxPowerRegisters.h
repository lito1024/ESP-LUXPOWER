#ifndef LUXPOWER_REGISTERS_H
#define LUXPOWER_REGISTERS_H

#include <stdint.h>

enum class LuxPowerRegisterSpace : uint8_t
{
    Input,
    Holding
};

enum class LuxPowerRegisterValueType : uint8_t
{
    U16,
    U32,
    Bit
};

struct LuxPowerRegisterDescriptor
{
    const char *name;
    const char *unit;
    LuxPowerRegisterSpace space;
    LuxPowerRegisterValueType type;
    uint16_t address;
    uint8_t bit;
    float scale;
};

enum class LuxPowerRegisterId : uint8_t
{
    Pv1EnergyTotal,
    FeedInGridEnabled,
    Pv1Power,
    Pv2Power,
    BatterySoc,
    BatteryChargePower,
    BatteryDischargePower
};

bool luxPowerRegisterDescriptor(LuxPowerRegisterId id, LuxPowerRegisterDescriptor &descriptor);

#endif
