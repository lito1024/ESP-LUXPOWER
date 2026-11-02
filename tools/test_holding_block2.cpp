// ESP-030 host test: LuxPowerTCP holding block 2 (registers 125..249).
// Builds LuxPowerTCP.cpp without ARDUINO against a scripted fake transport
// and a fake clock. Run via tools/run_holding_block2_tests.py.
#include "../lib/LuxPowerTCP/src/LuxPowerTCP.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <string>
#include <vector>

static uint32_t g_now = 1;
uint32_t luxPowerMillis() { return g_now; }
void luxPowerDelay(uint32_t ms) { g_now += ms; }
void luxPowerLog(const char *) {}
void luxPowerLogf(const char *, ...) {}
void luxPowerLogHex(const uint8_t *, size_t) {}

static int g_failures = 0;
#define CHECK(cond)                                                       \
    do                                                                    \
    {                                                                     \
        if (!(cond))                                                      \
        {                                                                 \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
            ++g_failures;                                                 \
        }                                                                 \
    } while (0)

static uint16_t crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i)
    {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc & 1) ? static_cast<uint16_t>((crc >> 1) ^ 0xA001) : static_cast<uint16_t>(crc >> 1);
    }
    return crc;
}

struct Request
{
    uint8_t function;
    uint16_t start;
    uint16_t count;
};

// Answers every read request with a well-formed frame whose register values
// are value(space, address). `answerStartOverride` >= 0 answers with a wrong
// start register (a frame the library must skip); `silent` answers nothing.
class FakeTransport : public LuxPowerTransport
{
public:
    std::vector<Request> requests;
    int answerStartOverride = -1;
    bool silent = false;
    uint16_t (*value)(uint8_t function, uint16_t address) = nullptr;

    bool connect(const char *, uint16_t) override { return open_ = true; }
    void disconnect() override { open_ = false; }
    bool connected() const override { return open_; }
    void flush() override {}
    int available() override { return static_cast<int>(rx_.size()); }
    int read(uint8_t *buffer, size_t length) override
    {
        size_t n = 0;
        while (n < length && !rx_.empty())
        {
            buffer[n++] = rx_.front();
            rx_.pop_front();
        }
        return static_cast<int>(n);
    }
    size_t write(const uint8_t *data, size_t length) override
    {
        if (length < 38)
            return length;
        const Request r{data[21], static_cast<uint16_t>(data[32] | (data[33] << 8)),
                        static_cast<uint16_t>(data[34] | (data[35] << 8))};
        requests.push_back(r);
        if (!silent)
            answer(r);
        return length;
    }

private:
    bool open_ = false;
    std::deque<uint8_t> rx_;

    void answer(const Request &r)
    {
        const uint16_t start = answerStartOverride >= 0 ? static_cast<uint16_t>(answerStartOverride) : r.start;
        const size_t total = 35 + 2u * r.count + 2;
        std::vector<uint8_t> f(total, 0);
        f[0] = 0xA1;
        f[1] = 0x1A;
        f[2] = 2;
        f[4] = static_cast<uint8_t>((total - 6) & 0xFF);
        f[5] = static_cast<uint8_t>((total - 6) >> 8);
        f[20] = 1;
        f[21] = r.function;
        f[32] = static_cast<uint8_t>(start & 0xFF);
        f[33] = static_cast<uint8_t>(start >> 8);
        f[34] = static_cast<uint8_t>(2 * r.count);
        for (uint16_t i = 0; i < r.count; ++i)
        {
            const uint16_t v = value ? value(r.function, static_cast<uint16_t>(r.start + i)) : 0;
            f[35 + 2 * i] = static_cast<uint8_t>(v & 0xFF);
            f[36 + 2 * i] = static_cast<uint8_t>(v >> 8);
        }
        const uint16_t crc = crc16(&f[20], total - 22);
        f[total - 2] = static_cast<uint8_t>(crc & 0xFF);
        f[total - 1] = static_cast<uint8_t>(crc >> 8);
        rx_.insert(rx_.end(), f.begin(), f.end());
    }
};

// Holding register N reads back as 1000 + N, except 168 (AC charge current) = 10 A.
static uint16_t registerValue(uint8_t function, uint16_t address)
{
    if (function == 0x03)
        return address == 168 ? 10 : static_cast<uint16_t>(1000 + address);
    return static_cast<uint16_t>(address);
}

static void pollUntilAllValid(LuxPowerTCP &lux)
{
    for (int i = 0; i < 4; ++i)
        lux.poll();
}

static void test_block2_read_and_cache()
{
    FakeTransport t;
    t.value = registerValue;
    LuxPowerTCP lux(t);
    lux.begin("dongle", 8000, "DONGLESN01", "INVERTSN01");

    uint16_t v = 0;
    lux.poll();  // input block 1
    lux.poll();  // input block 2
    lux.poll();  // holding block 1
    CHECK(lux.holdingCacheValid());
    CHECK(!lux.holdingBlock2CacheValid());
    CHECK(!lux.cachedHoldingU16(168, v));  // block 1 alone never serves 125..249
    CHECK(lux.cachedHoldingU16(124, v) && v == 1124);

    lux.poll();  // holding block 2
    CHECK(lux.holdingBlock2CacheValid());
    CHECK(lux.cachedHoldingU16(168, v) && v == 10);
    CHECK(lux.cachedHoldingU16(125, v) && v == 1125);
    CHECK(lux.cachedHoldingU16(249, v) && v == 1249);
    CHECK(!lux.cachedHoldingU16(250, v));
    CHECK(lux.cachedHoldingU16(21, v) && v == 1021);  // block 1 unchanged

    CHECK(t.requests.size() == 4);
    CHECK(t.requests[2].function == 0x03 && t.requests[2].start == 0 && t.requests[2].count == 125);
    CHECK(t.requests[3].function == 0x03 && t.requests[3].start == 125 && t.requests[3].count == 125);
}

static void test_poll_schedule()
{
    FakeTransport t;
    t.value = registerValue;
    LuxPowerTCP lux(t);
    lux.begin("dongle", 8000, "DONGLESN01", "INVERTSN01");
    lux.setInputPollInterval(1000000);
    lux.setInputBlock2PollInterval(1000000);
    lux.setHoldingPollInterval(30000);
    pollUntilAllValid(lux);
    const size_t before = t.requests.size();
    CHECK(!lux.poll());  // nothing due
    CHECK(t.requests.size() == before);

    g_now += 30000;  // holding interval: block 1 first, then block 2
    lux.poll();
    lux.poll();
    CHECK(t.requests.size() == before + 2);
    CHECK(t.requests[before].start == 0 && t.requests[before + 1].start == 125);
    CHECK(lux.holdingBlock2CacheAge() < 30000);
}

static void test_wrong_or_missing_answer_keeps_block2_invalid()
{
    FakeTransport t;
    t.value = registerValue;
    LuxPowerTCP lux(t);
    lux.begin("dongle", 8000, "DONGLESN01", "INVERTSN01");
    lux.poll();
    lux.poll();
    lux.poll();

    uint16_t v = 0;
    t.answerStartOverride = 0;  // answers as if for holding 0..124 -> must be skipped
    CHECK(!lux.poll());
    CHECK(!lux.holdingBlock2CacheValid());
    CHECK(!lux.cachedHoldingU16(168, v));

    t.answerStartOverride = -1;
    t.silent = true;  // dongle stops answering -> timeout, no hang, still invalid
    const uint32_t started = g_now;
    CHECK(!lux.poll());
    CHECK(g_now - started <= 4000);
    CHECK(!lux.holdingBlock2CacheValid());
    CHECK(std::string(lux.lastError()) == "Response timeout");
}

int main()
{
    test_block2_read_and_cache();
    test_poll_schedule();
    test_wrong_or_missing_answer_keeps_block2_invalid();
    if (g_failures)
    {
        std::printf("holding block 2: %d check(s) failed\n", g_failures);
        return 1;
    }
    std::printf("holding block 2: all checks passed\n");
    return 0;
}
