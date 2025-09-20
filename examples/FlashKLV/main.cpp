#include <Arduino.h>

#include <FlashKLV.h>

#include <array>
#include <boards/pico.h>
#include <hardware/flash.h>
#include <hardware/sync.h>

extern char __flash_binary_start;  // defined in linker script
extern char __flash_binary_end;    // defined in linker script
const uintptr_t codeStart = (uintptr_t) &__flash_binary_start;
const uintptr_t codeEnd = (uintptr_t) &__flash_binary_end;
const uintptr_t codeSize = codeEnd - codeStart;

// create a FlashKLV object
enum { SECTOR_COUNT = 4 };
static FlashKLV flashKLV(SECTOR_COUNT);

enum { CONFIG_KEY_A = 0x0A };
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
    while(!Serial); // wait for Serial to initialize

    delay(200);

    Serial.println();
    Serial.println("Code start: 0x" + String(codeStart, HEX));
    Serial.println("Code end:   0x" + String(codeEnd, HEX));
    Serial.println("Code size:  0x" + String(codeSize, HEX) + "("+String(codeSize, DEC) + ")");
    Serial.println();

    const uint32_t sectorsAvailable = (PICO_FLASH_SIZE_BYTES - codeSize) / FLASH_SECTOR_SIZE;
    Serial.println("Number of sectors available for FlashKLV: 0x" + String(sectorsAvailable, HEX) + "("+String(sectorsAvailable, DEC) + ")");
    Serial.println();

    Serial.println("FlashPtr: 0x" + String(reinterpret_cast<uint32_t>(flashKLV.getCurrentBankMemoryPtr()), HEX));
    Serial.println("FlashPtr-XIP: 0x" + String(reinterpret_cast<uint32_t>(flashKLV.getCurrentBankMemoryPtr()) - XIP_BASE, HEX));
    Serial.println("Offset: 0x" + String(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE*SECTOR_COUNT, HEX));
    Serial.println();

    Serial.println("FlashSize: 0x" + String(PICO_FLASH_SIZE_BYTES, HEX));

    flashKLV.eraseSector(0);

    enum { MEMORY_DISPLAY_SIZE = 36 };
    printBuf(flashKLV.getCurrentBankMemoryPtr(), MEMORY_DISPLAY_SIZE);

    // stop writing
    if (flashKLV.findFirstFreePos() < 24) {
        const config_t configA1 = { .a = 0xAAAA, .b =0xBB, .c = 0xCC };
        const config_t configA2 = { .a = 0xCCCC, .b =0xDD, .c = 0xEE };
        const config_t configB  = { .a = 0x1111, .b =0x22, .c = 0x33 };

        delay(100);
        Serial.println("****WRITE****");
        int32_t err = flashKLV.write(CONFIG_KEY_A, sizeof(configA1), &configA1);
        Serial.println("write err = " + String(err, DEC));
        printBuf(flashKLV.getCurrentBankMemoryPtr(), MEMORY_DISPLAY_SIZE);

        delay(100);
        // write configA2, overwriting config A1
        Serial.println("****WRITE****");
        err = flashKLV.write(CONFIG_KEY_A, sizeof(configA2), &configA2);
        Serial.println("write err = " + String(err, DEC));
        printBuf(flashKLV.getCurrentBankMemoryPtr(), MEMORY_DISPLAY_SIZE);

        delay(100);
        Serial.println("****WRITE****");
        err = flashKLV.write(CONFIG_KEY_B, sizeof(configB), &configB);
        Serial.println("write err = " + String(err, DEC));
        printBuf(flashKLV.getCurrentBankMemoryPtr(), MEMORY_DISPLAY_SIZE);

        delay(100);
        // write configA1, overwriting config A2
        Serial.println("****WRITE****");
        err = flashKLV.write(CONFIG_KEY_A, sizeof(configA1), &configA1);
        Serial.println("write err = " + String(err, DEC));
        printBuf(flashKLV.getCurrentBankMemoryPtr(), MEMORY_DISPLAY_SIZE);
    }

    delay(100);
    // read a config structure
    config_t configR {};
    Serial.println("****READ****");
    int32_t err = flashKLV.read(&configR, sizeof(configR), CONFIG_KEY_A);
    Serial.println("read err = " + String(err, DEC));
    Serial.println("record(0x0" + String(CONFIG_KEY_A, HEX) + ") " + String(configR.a, HEX) + " "+ String(configR.b, HEX) + " " + String(configR.c, HEX));

    Serial.println();
    Serial.println("****READ****");
    err = flashKLV.read(&configR, sizeof(configR), CONFIG_KEY_B);
    Serial.println("read err = " + String(err, DEC));
    Serial.println("record(0x0" + String(CONFIG_KEY_B, HEX) + ") " + String(configR.a, HEX) + " "+ String(configR.b, HEX) + " " + String(configR.c, HEX));
}

void loop()
{
    delay(100);
}