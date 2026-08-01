#pragma once

#include <memory>
#include <string>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/socket/socket.h"
#include "esphome/core/component.h"

#include "LuxPowerTCP.h"
#include "LuxPowerTransport.h"

namespace esphome {
namespace luxpower_tcp {

class LuxPowerESPHomeTransport : public LuxPowerTransport
{
public:
    bool connect(const char *host, uint16_t port) override;
    void disconnect() override;
    bool connected() const override;
    size_t write(const uint8_t *data, size_t length) override;
    void flush() override;
    int available() override;
    int read(uint8_t *buffer, size_t length) override;

private:
    std::unique_ptr<socket::Socket> socket_;
    bool connected_ = false;
};

class LuxPowerTCPComponent : public Component
{
public:
    void setup() override;
    void loop() override;
    void dump_config() override;

    void set_host(const std::string &host) { host_ = host; }
    void set_port(uint16_t port) { port_ = port; }
    void set_dongle_serial(const std::string &serial) { dongle_serial_ = serial; }
    void set_inverter_serial(const std::string &serial) { inverter_serial_ = serial; }
    void set_input_poll_interval(uint32_t interval_ms) { input_poll_interval_ms_ = interval_ms; }
    void set_holding_poll_interval(uint32_t interval_ms) { holding_poll_interval_ms_ = interval_ms; }

    void set_pv1_power_sensor(sensor::Sensor *sensor) { pv1_power_sensor_ = sensor; }
    void set_pv2_power_sensor(sensor::Sensor *sensor) { pv2_power_sensor_ = sensor; }
    void set_battery_soc_sensor(sensor::Sensor *sensor) { battery_soc_sensor_ = sensor; }
    void set_battery_charge_power_sensor(sensor::Sensor *sensor) { battery_charge_power_sensor_ = sensor; }
    void set_battery_discharge_power_sensor(sensor::Sensor *sensor) { battery_discharge_power_sensor_ = sensor; }
    void set_pv1_energy_total_sensor(sensor::Sensor *sensor) { pv1_energy_total_sensor_ = sensor; }
    void set_feed_in_grid_enabled_sensor(binary_sensor::BinarySensor *sensor) { feed_in_grid_enabled_sensor_ = sensor; }
    void set_tcp_connected_sensor(binary_sensor::BinarySensor *sensor) { tcp_connected_sensor_ = sensor; }

protected:
    void publish_metrics_();
    void publish_float_(sensor::Sensor *sensor, bool ok, float value);
    void publish_binary_(binary_sensor::BinarySensor *sensor, bool ok, bool value);

    LuxPowerESPHomeTransport transport_;
    LuxPowerTCP luxpower_{transport_};
    std::string host_;
    uint16_t port_ = 8000;
    std::string dongle_serial_;
    std::string inverter_serial_;
    uint32_t input_poll_interval_ms_ = 1000;
    uint32_t holding_poll_interval_ms_ = 30000;
    uint32_t last_publish_ms_ = 0;

    sensor::Sensor *pv1_power_sensor_ = nullptr;
    sensor::Sensor *pv2_power_sensor_ = nullptr;
    sensor::Sensor *battery_soc_sensor_ = nullptr;
    sensor::Sensor *battery_charge_power_sensor_ = nullptr;
    sensor::Sensor *battery_discharge_power_sensor_ = nullptr;
    sensor::Sensor *pv1_energy_total_sensor_ = nullptr;
    binary_sensor::BinarySensor *feed_in_grid_enabled_sensor_ = nullptr;
    binary_sensor::BinarySensor *tcp_connected_sensor_ = nullptr;
};

}  // namespace luxpower_tcp
}  // namespace esphome
