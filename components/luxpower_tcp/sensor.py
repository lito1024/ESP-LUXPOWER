import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_PERCENT,
    UNIT_WATT,
)

from . import LuxPowerTCPComponent

CONF_LUXPOWER_ID = "luxpower_id"
CONF_PV1_POWER = "pv1_power"
CONF_PV2_POWER = "pv2_power"
CONF_BATTERY_SOC = "battery_soc"
CONF_BATTERY_CHARGE_POWER = "battery_charge_power"
CONF_BATTERY_DISCHARGE_POWER = "battery_discharge_power"
CONF_PV1_ENERGY_TOTAL = "pv1_energy_total"

SENSOR_MAP = {
    CONF_PV1_POWER: ("set_pv1_power_sensor", UNIT_WATT, DEVICE_CLASS_POWER, STATE_CLASS_MEASUREMENT, 0),
    CONF_PV2_POWER: ("set_pv2_power_sensor", UNIT_WATT, DEVICE_CLASS_POWER, STATE_CLASS_MEASUREMENT, 0),
    CONF_BATTERY_SOC: ("set_battery_soc_sensor", UNIT_PERCENT, DEVICE_CLASS_BATTERY, STATE_CLASS_MEASUREMENT, 0),
    CONF_BATTERY_CHARGE_POWER: ("set_battery_charge_power_sensor", UNIT_WATT, DEVICE_CLASS_POWER, STATE_CLASS_MEASUREMENT, 0),
    CONF_BATTERY_DISCHARGE_POWER: ("set_battery_discharge_power_sensor", UNIT_WATT, DEVICE_CLASS_POWER, STATE_CLASS_MEASUREMENT, 0),
    CONF_PV1_ENERGY_TOTAL: ("set_pv1_energy_total_sensor", "kWh", DEVICE_CLASS_ENERGY, STATE_CLASS_TOTAL_INCREASING, 1),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LUXPOWER_ID): cv.use_id(LuxPowerTCPComponent),
        **{
            cv.Optional(key): sensor.sensor_schema(
                unit_of_measurement=unit,
                device_class=device_class,
                state_class=state_class,
                accuracy_decimals=accuracy,
            )
            for key, (_, unit, device_class, state_class, accuracy) in SENSOR_MAP.items()
        },
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LUXPOWER_ID])

    for key, (setter, *_metadata) in SENSOR_MAP.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(parent, setter)(sens))
