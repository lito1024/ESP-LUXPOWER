import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, DEVICE_CLASS_CONNECTIVITY

from . import LuxPowerTCPComponent

CONF_LUXPOWER_ID = "luxpower_id"
CONF_FEED_IN_GRID_ENABLED = "feed_in_grid_enabled"
CONF_TCP_CONNECTED = "tcp_connected"

BINARY_SENSOR_MAP = {
    CONF_FEED_IN_GRID_ENABLED: ("set_feed_in_grid_enabled_sensor", None),
    CONF_TCP_CONNECTED: ("set_tcp_connected_sensor", DEVICE_CLASS_CONNECTIVITY),
}


def luxpower_binary_sensor_schema(device_class):
    if device_class is None:
        return binary_sensor.binary_sensor_schema()
    return binary_sensor.binary_sensor_schema(device_class=device_class)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LUXPOWER_ID): cv.use_id(LuxPowerTCPComponent),
        **{
            cv.Optional(key): luxpower_binary_sensor_schema(device_class)
            for key, (_, device_class) in BINARY_SENSOR_MAP.items()
        },
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LUXPOWER_ID])

    for key, (setter, _device_class) in BINARY_SENSOR_MAP.items():
        if key in config:
            sens = await binary_sensor.new_binary_sensor(config[key])
            cg.add(getattr(parent, setter)(sens))
