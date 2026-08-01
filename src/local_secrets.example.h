#ifndef LUXPOWER_LOCAL_SECRETS_EXAMPLE_H
#define LUXPOWER_LOCAL_SECRETS_EXAMPLE_H

#include <Arduino.h>

// Copy this file to src/local_secrets.h and fill in your local values.
// src/local_secrets.h is ignored by Git and should not be committed.

// Wi-Fi network used by the ESP32 test application.
#define LUXPOWER_WIFI_SSID "YOUR_WIFI_SSID"
#define LUXPOWER_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// LuxPower dongle TCP endpoint.
// Use the LAN IP address of the LuxPower dongle.
#define LUXPOWER_HOST_IP IPAddress(192, 0, 2, 10)
#define LUXPOWER_TCP_PORT 8000

// LuxPower protocol serials.
// These are the dongle serial and inverter serial used by LuxPowerTCP.begin().
#define LUXPOWER_DONGLE_SERIAL "YOUR_DONGLE_SERIAL"
#define LUXPOWER_INVERTER_SERIAL "YOUR_INVERTER_SERIAL"

#endif
