#include <FlashKLV.h>
#include <unity.h>

#include <array>


void setUp()
{
}

void tearDown()
{
}


// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-reinterpret-cast,hicpp-signed-bitwise,readability-identifier-length,readability-magic-numbers,altera-struct-pack-align)
enum { SECTOR_COUNT = 2 };
static std::array<uint8_t, FlashKLV::SECTOR_SIZE*SECTOR_COUNT> flashMemory;

// for FRAMEWORK_TEST, PAGE_SIZE = 8

enum : uint8_t { TOP_BIT = 0x80, WORD_BIT = 0x40, TOP_BITS = TOP_BIT|WORD_BIT };

struct record3_t {
    enum { LENGTH = 3 };
    uint16_t key = 0x13;
    uint16_t length = LENGTH;
    std::array<uint8_t, LENGTH> value {};
};

struct record4_t {
    uint16_t key = 0x14;
    uint16_t length = sizeof(int32_t);
    int32_t value = 0;
};

struct record7_t {
    enum { LENGTH = 7 };
    uint16_t key = 0x17;
    uint16_t length = LENGTH;
    std::array<uint8_t, LENGTH> value {};
};

struct record12_t {
    enum { LENGTH = 12 };
    uint16_t key = 0x12;
    uint16_t length = LENGTH;
    std::array<uint8_t, LENGTH> value {};
};

struct record23_t {
    enum { LENGTH = 23 };
    uint16_t key = 0x23;
    uint16_t length = LENGTH;
    std::array<uint8_t, LENGTH> value {};
};

void test_klv8_keys()
{
    flashMemory.fill(0xFF);
    FlashKLV flashKLV(&flashMemory[0], FlashKLV::SECTOR_SIZE*SECTOR_COUNT);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8));

    TEST_ASSERT_EQUAL(false, FlashKLV::keyOK(0x00));

    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(0x01));
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(0x3E));
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(0x3F));

    TEST_ASSERT_EQUAL(false, FlashKLV::keyOK(0x40));
    TEST_ASSERT_EQUAL(false, FlashKLV::keyOK(0x41));
    TEST_ASSERT_EQUAL(false, FlashKLV::keyOK(0xFE));
    TEST_ASSERT_EQUAL(false, FlashKLV::keyOK(0xFF));

    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(0x0100));
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(0x0101));
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(0x3FFE));
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(0x3FFF));

    TEST_ASSERT_EQUAL(false, FlashKLV::keyOK(0x4000));
    TEST_ASSERT_EQUAL(false, FlashKLV::keyOK(0x4001));
    TEST_ASSERT_EQUAL(false, FlashKLV::keyOK(0xFFFE));
    TEST_ASSERT_EQUAL(false, FlashKLV::keyOK(0xFFFF));

    const record3_t record3A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record3A.key));
    TEST_ASSERT_EQUAL(0, record3A.value[0]);

    const record4_t record4A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record4A.key));
    TEST_ASSERT_EQUAL(0, record4A.value);

    const record7_t record7A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record7A.key));
    TEST_ASSERT_EQUAL(0, record7A.value[0]);

    const record12_t record12A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record12A.key));
    TEST_ASSERT_EQUAL(0, record12A.value[0]);

    const record23_t record23A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record23A.key));
    TEST_ASSERT_EQUAL(0, record23A.value[0]);
}

void test_klv8()
{
    flashMemory.fill(0xFF);
    FlashKLV flashKLV(&flashMemory[0], FlashKLV::SECTOR_SIZE*SECTOR_COUNT);

    FlashKLV::klv_t klv {};
    int err {};
    uint32_t value {};
    record4_t record4A {};

    TEST_ASSERT_EQUAL(flashKLV.memorySize(), flashKLV.bytesFree());

    TEST_ASSERT_EQUAL(6, record4A.length + sizeof(FlashKLV::kl8_t));
    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_TRUE(klv.key == FlashKLV::NOT_FOUND);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(0));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));
    TEST_ASSERT_EQUAL(FlashKLV::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.valuePtr);

    record4A.value = 0x7B536AFE;
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(flashKLV.memorySize() - 6, flashKLV.bytesFree());

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(5));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(6)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(7)); // check byte after record still set to 0xFF
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(6));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(6));

    TEST_ASSERT_EQUAL(false, flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(0x14, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(4, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(6, flashKLV.getRecordPositionIncrement(0));
    TEST_ASSERT_EQUAL_PTR(&flashMemory[2], flashKLV.getRecordValuePtr(0));

    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_EQUAL(record4A.key, klv.key);
    TEST_ASSERT_EQUAL(record4A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(2), klv.valuePtr);
    value = *reinterpret_cast<const uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(record4A.value, value);

    // write the same value again, so should not be written and be in same position
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_NO_NEED_TO_WRITE, err);

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(5));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(6)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(7));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(6));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(6));

    TEST_ASSERT_EQUAL(0x14, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(4, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(6, flashKLV.getRecordPositionIncrement(0));
    TEST_ASSERT_EQUAL_PTR(flashKLV.flashPos(2), flashKLV.getRecordValuePtr(0));

    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_EQUAL(record4A.key, klv.key);
    TEST_ASSERT_EQUAL(record4A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(2), klv.valuePtr);
    value = *reinterpret_cast<const uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(record4A.value, value);

    // a value of 0x7B536A00 will allow the record to be overwritten
    record4A.value = 0x7B536A00;
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_OVERWRITTEN, err);
    TEST_ASSERT_EQUAL(flashKLV.memorySize() - 6, flashKLV.bytesFree());

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(5));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(6)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(7));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(6));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(6));

    TEST_ASSERT_EQUAL(0x14, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(4, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(6, flashKLV.getRecordPositionIncrement(0));
    TEST_ASSERT_EQUAL_PTR(&flashMemory[2], flashKLV.getRecordValuePtr(0));

    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_EQUAL(record4A.key, klv.key);
    TEST_ASSERT_EQUAL(record4A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(2), klv.valuePtr);
    value = *reinterpret_cast<const uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(record4A.value, value);

    // a value of 0x0C0B0AFE will will require the record to be marked as deleted and a new record written
    record4A.value = 0x0C0B0AFE;
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(flashKLV.memorySize() - 12, flashKLV.bytesFree());

    // old first record, marked as deleted
    TEST_ASSERT_EQUAL(0x14, flashKLV.flashPeek(0)); // first has top bit cleared to mark as delete
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(5));

    // new record
    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(0x0B, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0x0C, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(12)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(13));
    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_EQUAL(record4A.key, klv.key);
    TEST_ASSERT_EQUAL(record4A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(8), klv.valuePtr);
    value = *reinterpret_cast<const uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(record4A.value, value);
}

void test_config()
{
    flashMemory.fill(0xFF);
    FlashKLV flashKLV(&flashMemory[0], FlashKLV::SECTOR_SIZE*SECTOR_COUNT);

    // declare a key and structure
    enum { CONFIG_KEY = 0x01 };
    struct config_t {
        uint16_t a;
        uint8_t b;
        uint8_t c;
    };

    // write the config structure to flash
    const config_t configW = { .a= 713, .b =27, .c = 12 };
    int32_t err = flashKLV.write(CONFIG_KEY, sizeof(configW), &configW);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    // read a config structure
    config_t configR {};
    err = flashKLV.read(&configR, sizeof(configR), CONFIG_KEY);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    // test the values are as expected
    TEST_ASSERT_EQUAL(713, configR.a);
    TEST_ASSERT_EQUAL(27, configR.b);
    TEST_ASSERT_EQUAL(12, configR.c);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-reinterpret-cast,hicpp-signed-bitwise,readability-identifier-length,readability-magic-numbers,altera-struct-pack-align)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_klv8_keys);
    RUN_TEST(test_klv8);
    RUN_TEST(test_config);

    UNITY_END();
}
