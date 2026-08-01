#include <Arduino.h>
#include <WiFi.h>
#include "LuxPowerTCP.h"

#if defined(__has_include)
#if __has_include("local_secrets.h")
#include "local_secrets.h"
#define LUXPOWER_LOCAL_SECRETS_LOADED 1
#else
#define LUXPOWER_LOCAL_SECRETS_LOADED 0
#endif
#else
#define LUXPOWER_LOCAL_SECRETS_LOADED 0
#endif

#ifndef LUXPOWER_WIFI_SSID
#define LUXPOWER_WIFI_SSID "CHANGE_ME"
#endif

#ifndef LUXPOWER_WIFI_PASSWORD
#define LUXPOWER_WIFI_PASSWORD "CHANGE_ME"
#endif

#ifndef LUXPOWER_HOST_IP
#define LUXPOWER_HOST_IP IPAddress(192, 168, 1, 100)
#endif

#ifndef LUXPOWER_DONGLE_SERIAL
#define LUXPOWER_DONGLE_SERIAL "CHANGE_ME"
#endif

#ifndef LUXPOWER_INVERTER_SERIAL
#define LUXPOWER_INVERTER_SERIAL "CHANGE_ME"
#endif

#ifndef LUXPOWER_TCP_PORT
#define LUXPOWER_TCP_PORT 8000
#endif

const char *WIFI_SSID = LUXPOWER_WIFI_SSID;
const char *WIFI_PASSWORD = LUXPOWER_WIFI_PASSWORD;

IPAddress LUXPOWER_IP = LUXPOWER_HOST_IP;
constexpr uint16_t LUXPOWER_PORT = LUXPOWER_TCP_PORT;
constexpr char DONGLE_SERIAL[] = LUXPOWER_DONGLE_SERIAL;
constexpr char INVERTER_SERIAL[] = LUXPOWER_INVERTER_SERIAL;
constexpr uint32_t WIFI_TIMEOUT_MS = 20000;
constexpr bool ENABLE_FULL_REGISTER_DUMP = false;
constexpr bool ENABLE_CHANGED_REGISTER_SCAN = false;

WiFiClient luxClient;
LuxPowerTCP luxPower(luxClient);

bool connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }

    Serial.printf("Подключение к Wi-Fi: %s\n", WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const uint32_t startedAt = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - startedAt >= WIFI_TIMEOUT_MS)
        {
            Serial.println("\nОшибка подключения к Wi-Fi.");
            return false;
        }

        Serial.print('.');
        delay(500);
    }

    Serial.println();
    Serial.println("Wi-Fi подключен.");
    Serial.print("IP ESP32: ");
    Serial.println(WiFi.localIP());

    return true;
}

void printLocalSecretsWarning()
{
#if !LUXPOWER_LOCAL_SECRETS_LOADED
    Serial.println("************************************************");
    Serial.println("local_secrets.h not found");
    Serial.println("Using placeholder configuration");
    Serial.println("Create src/local_secrets.h from");
    Serial.println("src/local_secrets.example.h");
    Serial.println("************************************************");
#endif
}

void setup()
{
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" LuxPower TCP test application");
    Serial.println("================================");
    printLocalSecretsWarning();

    if (!connectWiFi())
    {
        return;
    }

    luxPower.begin(LUXPOWER_IP, LUXPOWER_PORT, DONGLE_SERIAL, INVERTER_SERIAL);
}

namespace
{
void printMetricValue(const LuxPowerTCP &device, LuxPowerMetric metric)
{
    LuxPowerMetricDescriptor descriptor{};
    if (!device.metricDescriptor(metric, descriptor))
    {
        Serial.println("Metric descriptor unavailable");
        return;
    }

    LuxPowerMetricValue value{};
    if (!device.metricValue(metric, value))
    {
        Serial.printf("%s: unavailable\n", descriptor.name);
        return;
    }

    if (value.type == LuxPowerMetricValue::Type::Float)
    {
        Serial.printf("%s: %.1f %s\n", descriptor.name, value.floatValue, descriptor.unit);
    }
    else
    {
        Serial.printf("%s: %s\n", descriptor.name, value.boolValue ? "ON" : "OFF");
    }
}

void printStage13ACandidateMetrics(const LuxPowerTCP &device)
{
    if (!device.inputCacheValid())
    {
        Serial.println("Stage 13A candidate metrics unavailable: input cache invalid.");
        return;
    }

    uint16_t rawBatteryCapacity = 0;
    uint16_t rawBmsMaxCellTemperature = 0;
    uint16_t rawPowerToGrid = 0;
    uint16_t rawPowerFromGrid = 0;

    const bool haveBatteryCapacity = device.cachedInputU16(97, rawBatteryCapacity);
    const bool haveBmsMaxCellTemperature = device.cachedInputU16(103, rawBmsMaxCellTemperature);
    const bool havePowerToGrid = device.cachedInputU16(26, rawPowerToGrid);
    const bool havePowerFromGrid = device.cachedInputU16(27, rawPowerFromGrid);

    Serial.println();
    Serial.println("===== STAGE 13A CANDIDATE METRICS =====");

    if (haveBatteryCapacity)
    {
        Serial.printf("raw Input[97] = %u\n", rawBatteryCapacity);
        Serial.printf("Candidate Battery Capacity: %.1f Ah\n", static_cast<float>(rawBatteryCapacity));
    }

    if (haveBmsMaxCellTemperature)
    {
        const float temperatureCelsius = static_cast<float>(rawBmsMaxCellTemperature) * 0.1f;
        Serial.printf("raw Input[103] = %u\n", rawBmsMaxCellTemperature);
        Serial.printf("Candidate BMS Max Cell Temperature: %.1f °C\n", temperatureCelsius);
        Serial.println("Candidate BMS Max Cell Temperature signed caveat: upstream documents signed, but does not sign-convert before scaling.");
    }

    if (havePowerToGrid)
    {
        Serial.printf("raw Input[26] = %u\n", rawPowerToGrid);
        Serial.printf("Candidate Power To Grid: %.1f W\n", static_cast<float>(rawPowerToGrid));
    }

    if (havePowerFromGrid)
    {
        Serial.printf("raw Input[27] = %u\n", rawPowerFromGrid);
        Serial.printf("Candidate Power From Grid: %.1f W\n", static_cast<float>(rawPowerFromGrid));
    }

    if (havePowerToGrid && havePowerFromGrid)
    {
        const int32_t gridFlow = static_cast<int32_t>(rawPowerFromGrid) - static_cast<int32_t>(rawPowerToGrid);
        Serial.printf("Candidate Grid Flow: %ld W\n", static_cast<long>(gridFlow));
        Serial.println("Candidate Grid Flow sign: positive = import, negative = export.");
    }
}
}  // namespace

void loop()
{
    static bool lastConnectedState = false;
    static bool scannerBaselinesCaptured = false;
    static bool fullDumpPrinted = false;

    luxPower.poll();
    const bool connectedNow = luxPower.connected();

    if (connectedNow != lastConnectedState)
    {
        lastConnectedState = connectedNow;
        Serial.printf("Connection state: %s\n", connectedNow ? "connected" : "disconnected");
    }

    const bool cachesReady = luxPower.inputCacheValid() && luxPower.holdingCacheValid();

    if (cachesReady && ENABLE_FULL_REGISTER_DUMP && !fullDumpPrinted)
    {
        fullDumpPrinted = true;
        Serial.println();
        Serial.println("===== FULL INPUT REGISTER DUMP =====");
        luxPower.dumpInputRegisters(Serial);
        Serial.println();
        Serial.println("===== FULL HOLDING REGISTER DUMP =====");
        luxPower.dumpHoldingRegisters(Serial);
    }

    if (ENABLE_CHANGED_REGISTER_SCAN && cachesReady && !scannerBaselinesCaptured)
    {
        scannerBaselinesCaptured = true;
        Serial.println();
        Serial.println("===== REGISTER CHANGE BASELINES =====");
        luxPower.dumpChangedInputRegisters(Serial, 1);
        luxPower.dumpChangedHoldingRegisters(Serial, 1);
    }

    static uint32_t lastPrintMs = 0;
    static uint32_t lastHoldingPrintMs = 0;
    static uint32_t lastInputScanMs = 0;
    static uint32_t lastHoldingScanMs = 0;

    if (millis() - lastPrintMs >= 1000)
    {
        lastPrintMs = millis();
        printMetricValue(luxPower, LuxPowerMetric::Pv1EnergyTotal);
        printMetricValue(luxPower, LuxPowerMetric::Pv1Power);
        printMetricValue(luxPower, LuxPowerMetric::Pv2Power);
        printMetricValue(luxPower, LuxPowerMetric::BatterySoc);
        printMetricValue(luxPower, LuxPowerMetric::BatteryChargePower);
        printMetricValue(luxPower, LuxPowerMetric::BatteryDischargePower);
        printStage13ACandidateMetrics(luxPower);
    }

    if (millis() - lastHoldingPrintMs >= 5000)
    {
        lastHoldingPrintMs = millis();
        printMetricValue(luxPower, LuxPowerMetric::FeedInGridEnabled);
    }

    if (ENABLE_CHANGED_REGISTER_SCAN && cachesReady && scannerBaselinesCaptured && millis() - lastInputScanMs >= 2000)
    {
        lastInputScanMs = millis();
        Serial.println();
        Serial.println("===== CHANGED INPUT REGISTERS =====");
        luxPower.dumpChangedInputRegisters(Serial, 1);
    }

    if (ENABLE_CHANGED_REGISTER_SCAN && cachesReady && scannerBaselinesCaptured && millis() - lastHoldingScanMs >= 10000)
    {
        lastHoldingScanMs = millis();
        Serial.println();
        Serial.println("===== CHANGED HOLDING REGISTERS =====");
        luxPower.dumpChangedHoldingRegisters(Serial, 1);
    }
}
