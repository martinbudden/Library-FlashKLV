#include <Arduino.h>

#include <FlashKLV.h>

#include <array>
#include <boards/pico.h>
#include <hardware/flash.h>
#include <hardware/sync.h>

enum { SECTOR_COUNT = 4 };
static FlashKLV flashKLV(SECTOR_COUNT);

enum { CONFIG_KEY = 0x01 };
enum { CONFIG_KEY_B = 0x0B };
struct config_t {
    uint16_t a;
    uint8_t b;
    uint8_t c;
};

void printBuf(const uint8_t *buf, size_t len)
{
    Serial.println();
    uint32_t count = 0;
    for (size_t ii = 0; ii < len; ++ii) {
        if (buf[ii] < 0x10) {
            Serial.print("0");
        }
        Serial.print(String(buf[ii], HEX));
        ++count;
        if (count == 6) {
            Serial.println();
            count = 0;
        } else {
            Serial.print(" ");
        }
    }
    Serial.println("\r\n");
}

void setup()
{
    //stdio_init_all();
    Serial.begin(115200);
    while(!Serial);
    delay(1000);
    // create a FlashKLV object

    Serial.println("FlashPtr: 0x" + String(reinterpret_cast<uint32_t>(flashKLV.getCurrentBankMemoryPtr()), HEX));
    Serial.println("FlashPtr-XIP: 0x" + String(reinterpret_cast<uint32_t>(flashKLV.getCurrentBankMemoryPtr()) - XIP_BASE, HEX));
    Serial.println("Offset: 0x" + String(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE*SECTOR_COUNT, HEX));
    Serial.println();

    Serial.println("FlashSize: 0x" + String(PICO_FLASH_SIZE_BYTES, HEX));

    // flashKLV.eraseSector(0);
    printBuf(flashKLV.getCurrentBankMemoryPtr(), 36);

    delay(100);
    Serial.println("****WRITE****");
    const config_t configW = { .a = 0x0AFF, .b =0x12, .c = 0x35 };
    int32_t err = flashKLV.write(CONFIG_KEY, sizeof(configW), &configW);
    Serial.println("write err = " + String(err, DEC));
    printBuf(flashKLV.getCurrentBankMemoryPtr(), 36);

    delay(100);
    Serial.println("****WRITE****");
    const config_t configW2 = { .a = 0xFF00, .b =0x88, .c = 0x99 };
    err = flashKLV.write(CONFIG_KEY, sizeof(configW2), &configW2);
    Serial.println("write err = " + String(err, DEC));
    printBuf(flashKLV.getCurrentBankMemoryPtr(), 36);

    delay(100);
    Serial.println("****WRITE****");
    const config_t configWB = { .a = 0x1111, .b =0x22, .c = 0x33 };
    err = flashKLV.write(CONFIG_KEY_B, sizeof(configWB), &configWB);
    Serial.println("write err = " + String(err, DEC));
    printBuf(flashKLV.getCurrentBankMemoryPtr(), 36);

    delay(100);
    // read a config structure
    config_t configR {};
    Serial.println("****READ****");
    err = flashKLV.read(&configR, sizeof(configR), CONFIG_KEY);
    Serial.println("read err = " + String(err, DEC));
    Serial.println("record(0x0" + String(CONFIG_KEY, HEX) + ") " + String(configR.a, HEX) + " "+ String(configR.b, HEX) + " " + String(configR.c, HEX));

    Serial.println();
    Serial.println("****READ****");
    err = flashKLV.read(&configR, sizeof(configR), CONFIG_KEY_B);
    Serial.println("read err = " + String(err, DEC));
    Serial.println("record(0x0" + String(CONFIG_KEY_B, HEX) + ") " + String(configR.a, HEX) + " "+ String(configR.b, HEX) + " " + String(configR.c, HEX));
}

void loop()
{
}