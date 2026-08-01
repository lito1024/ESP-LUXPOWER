#include "luxpower_tcp_component.h"

#include <cerrno>
#include <cstdarg>
#include <cstdio>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace luxpower_tcp {

static const char *const TAG = "luxpower_tcp";

bool LuxPowerESPHomeTransport::connect(const char *host, uint16_t port)
{
    if (connected())
    {
        return true;
    }

    struct sockaddr_storage addr;
    const socklen_t addrlen = socket::set_sockaddr(reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr), host, port);
    if (addrlen == 0)
    {
        ESP_LOGW(TAG, "Invalid LuxPower host address: %s", host);
        return false;
    }

    const int family = reinterpret_cast<struct sockaddr *>(&addr)->sa_family;
    socket_ = socket::socket_loop_monitored(family, SOCK_STREAM, IPPROTO_TCP);
    if (!socket_)
    {
        ESP_LOGW(TAG, "Failed to create LuxPower TCP socket");
        return false;
    }

    socket_->setblocking(true);
    if (socket_->connect(reinterpret_cast<struct sockaddr *>(&addr), addrlen) != 0)
    {
        ESP_LOGW(TAG, "LuxPower TCP connect failed: errno=%d", errno);
        disconnect();
        return false;
    }

    socket_->setblocking(false);
    connected_ = true;
    return true;
}

void LuxPowerESPHomeTransport::disconnect()
{
    socket_.reset();
    connected_ = false;
}

bool LuxPowerESPHomeTransport::connected() const
{
    return socket_ != nullptr && connected_;
}

size_t LuxPowerESPHomeTransport::write(const uint8_t *data, size_t length)
{
    if (!connected())
    {
        return 0;
    }

    ssize_t written = socket_->write(data, length);
    if (written < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            ESP_LOGW(TAG, "LuxPower TCP write failed: errno=%d", errno);
            disconnect();
        }
        return 0;
    }

    return static_cast<size_t>(written);
}

void LuxPowerESPHomeTransport::flush()
{
}

int LuxPowerESPHomeTransport::available()
{
    return connected() && socket_->ready() ? 1 : 0;
}

int LuxPowerESPHomeTransport::read(uint8_t *buffer, size_t length)
{
    if (!connected())
    {
        return 0;
    }

    ssize_t received = socket_->read(buffer, length);
    if (received == 0)
    {
        disconnect();
        return 0;
    }
    if (received < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            ESP_LOGW(TAG, "LuxPower TCP read failed: errno=%d", errno);
            disconnect();
        }
        return 0;
    }

    return static_cast<int>(received);
}

void LuxPowerTCPComponent::setup()
{
    luxpower_.begin(host_.c_str(), port_, dongle_serial_.c_str(), inverter_serial_.c_str());
    luxpower_.setInputPollInterval(input_poll_interval_ms_);
    luxpower_.setHoldingPollInterval(holding_poll_interval_ms_);
}

void LuxPowerTCPComponent::loop()
{
    luxpower_.poll();

    const uint32_t now = millis();
    if (last_publish_ms_ == 0 || now - last_publish_ms_ >= 1000)
    {
        last_publish_ms_ = now;
        publish_metrics_();
    }
}

void LuxPowerTCPComponent::dump_config()
{
    ESP_LOGCONFIG(TAG, "LuxPower TCP:");
    ESP_LOGCONFIG(TAG, "  Host: %s:%u", host_.c_str(), port_);
    ESP_LOGCONFIG(TAG, "  Input poll interval: %u ms", static_cast<unsigned int>(input_poll_interval_ms_));
    ESP_LOGCONFIG(TAG, "  Holding poll interval: %u ms", static_cast<unsigned int>(holding_poll_interval_ms_));
}

void LuxPowerTCPComponent::publish_metrics_()
{
    float value = 0.0f;
    bool state = false;

    publish_float_(pv1_power_sensor_, luxpower_.pv1Power(value), value);
    publish_float_(pv2_power_sensor_, luxpower_.pv2Power(value), value);
    publish_float_(battery_soc_sensor_, luxpower_.batterySoc(value), value);
    publish_float_(battery_charge_power_sensor_, luxpower_.batteryChargePower(value), value);
    publish_float_(battery_discharge_power_sensor_, luxpower_.batteryDischargePower(value), value);
    publish_float_(pv1_energy_total_sensor_, luxpower_.pv1EnergyTotal(value), value);
    publish_binary_(feed_in_grid_enabled_sensor_, luxpower_.feedInGridEnabled(state), state);

    state = luxpower_.connected();
    publish_binary_(tcp_connected_sensor_, true, state);
}

void LuxPowerTCPComponent::publish_float_(sensor::Sensor *sensor, bool ok, float value)
{
    if (sensor != nullptr && ok)
    {
        sensor->publish_state(value);
    }
}

void LuxPowerTCPComponent::publish_binary_(binary_sensor::BinarySensor *sensor, bool ok, bool value)
{
    if (sensor != nullptr && ok)
    {
        sensor->publish_state(value);
    }
}

}  // namespace luxpower_tcp
}  // namespace esphome

uint32_t luxPowerMillis()
{
    return esphome::millis();
}

void luxPowerDelay(uint32_t milliseconds)
{
    esphome::delay(milliseconds);
}

void luxPowerLog(const char *message)
{
    ESP_LOGD("luxpower_tcp.library", "%s", message);
}

void luxPowerLogf(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    ESP_LOGD("luxpower_tcp.library", "%s", buffer);
}

void luxPowerLogHex(const uint8_t *data, size_t length)
{
    char line[97];
    size_t pos = 0;
    for (size_t i = 0; i < length && pos + 4 < sizeof(line); ++i)
    {
        pos += snprintf(line + pos, sizeof(line) - pos, "%02X ", data[i]);
    }
    line[pos] = '\0';
    ESP_LOGD("luxpower_tcp.library", "%s", line);
}
