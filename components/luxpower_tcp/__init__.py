import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PORT

DEPENDENCIES = ["network"]
AUTO_LOAD = ["socket"]
MULTI_CONF = False

luxpower_tcp_ns = cg.esphome_ns.namespace("luxpower_tcp")
LuxPowerTCPComponent = luxpower_tcp_ns.class_("LuxPowerTCPComponent", cg.Component)

CONF_DONGLE_SERIAL = "dongle_serial"
CONF_HOST = "host"
CONF_INVERTER_SERIAL = "inverter_serial"
CONF_INPUT_POLL_INTERVAL = "input_poll_interval"
CONF_HOLDING_POLL_INTERVAL = "holding_poll_interval"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LuxPowerTCPComponent),
        cv.Required(CONF_HOST): cv.string_strict,
        cv.Optional(CONF_PORT, default=8000): cv.port,
        cv.Required(CONF_DONGLE_SERIAL): cv.string_strict,
        cv.Required(CONF_INVERTER_SERIAL): cv.string_strict,
        cv.Optional(CONF_INPUT_POLL_INTERVAL, default="1s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_HOLDING_POLL_INTERVAL, default="30s"): cv.positive_time_period_milliseconds,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_port(config[CONF_PORT]))
    cg.add(var.set_dongle_serial(config[CONF_DONGLE_SERIAL]))
    cg.add(var.set_inverter_serial(config[CONF_INVERTER_SERIAL]))
    cg.add(var.set_input_poll_interval(config[CONF_INPUT_POLL_INTERVAL]))
    cg.add(var.set_holding_poll_interval(config[CONF_HOLDING_POLL_INTERVAL]))
