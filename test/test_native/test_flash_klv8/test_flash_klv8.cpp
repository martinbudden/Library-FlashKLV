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
static std::array<uint8_t, FlashKlv::SECTOR_SIZE*SECTOR_COUNT> flashMemory;

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
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8));

    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0x00));

    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(0x01));
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(0x3E));
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(0x3F));

    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0x40));
    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0x41));
    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0xFE));
    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0xFF));

    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(0x0100));
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(0x0101));
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(0x3FFC));
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(0x3FFD));
    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0x3FFE));
    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0x3FFF));

    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0x4000));
    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0x4001));
    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0xFFFE));
    TEST_ASSERT_EQUAL(false, FlashKlv::key_ok(0xFFFF));

    const record3_t record3A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record3A.key));
    TEST_ASSERT_EQUAL(0, record3A.value[0]);

    const record4_t record4A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record4A.key));
    TEST_ASSERT_EQUAL(0, record4A.value);

    const record7_t record7A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record7A.key));
    TEST_ASSERT_EQUAL(0, record7A.value[0]);

    const record12_t record12A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record12A.key));
    TEST_ASSERT_EQUAL(0, record12A.value[0]);

    const record23_t record23A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record23A.key));
    TEST_ASSERT_EQUAL(0, record23A.value[0]);
}

void test_klv8()
{
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();

    FlashKlv::klv_t klv {};
    int err {};
    uint32_t value {};
    record4_t record4A {};

    TEST_ASSERT_EQUAL(flashKLV.memory_size(), flashKLV.bytes_free());

    TEST_ASSERT_EQUAL(6, record4A.length + sizeof(FlashKlv::kl8_t));
    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_TRUE(klv.key == FlashKlv::NOT_FOUND);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(0));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));
    TEST_ASSERT_EQUAL(FlashKlv::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.value_ptr);

    record4A.value = 0x7B536AFE;
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(flashKLV.memory_size() - 6, flashKLV.bytes_free());

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(5));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(6)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7)); // check byte after record still set to 0xFF
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(6));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(6));

    TEST_ASSERT_EQUAL(false, flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(0x14, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(4, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(6, flashKLV.get_record_position_increment(0));
    TEST_ASSERT_EQUAL_PTR(&flashMemory[2], flashKLV.get_record_value_ptr(0));

    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_EQUAL(record4A.key, klv.key);
    TEST_ASSERT_EQUAL(record4A.length, klv.length);
     //!!TEST_ASSERT_EQUAL(flashKLV.flash_pos(2), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(record4A.value, value);

    // write the same value again, so should not be written and be in same position
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_NO_NEED_TO_WRITE, err);

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(5));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(6)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(6));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(6));

    TEST_ASSERT_EQUAL(0x14, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(4, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(6, flashKLV.get_record_position_increment(0));
    //!!TEST_ASSERT_EQUAL_PTR(flashKLV.flash_pos(2), flashKLV.get_record_value_ptr(0));

    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_EQUAL(record4A.key, klv.key);
    TEST_ASSERT_EQUAL(record4A.length, klv.length);
     //!!TEST_ASSERT_EQUAL(flashKLV.flash_pos(2), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(record4A.value, value);

    // a value of 0x7B536A00 will allow the record to be overwritten
    record4A.value = 0x7B536A00;
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_OVERWRITTEN, err);
    TEST_ASSERT_EQUAL(flashKLV.memory_size() - 6, flashKLV.bytes_free());

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(5));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(6)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(6));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(6));

    TEST_ASSERT_EQUAL(0x14, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(4, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(6, flashKLV.get_record_position_increment(0));
    TEST_ASSERT_EQUAL_PTR(&flashMemory[2], flashKLV.get_record_value_ptr(0));

    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_EQUAL(record4A.key, klv.key);
    TEST_ASSERT_EQUAL(record4A.length, klv.length);
    //!!TEST_ASSERT_EQUAL(flashKLV.flash_pos(2), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(record4A.value, value);

    // a value of 0x0C0B0AFE will will require the record to be marked as deleted and a new record written
    record4A.value = 0x0C0B0AFE;
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(flashKLV.memory_size() - 12, flashKLV.bytes_free());

    // old first record, marked as deleted
    TEST_ASSERT_EQUAL(0x14, flashKLV.flash_peek(0)); // first has top bit cleared to mark as delete
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(5));

    // new record
    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(0x0B, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0x0C, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(12)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(13));
    klv = flashKLV.find(record4A.key);
    TEST_ASSERT_EQUAL(record4A.key, klv.key);
    TEST_ASSERT_EQUAL(record4A.length, klv.length);
    //!!TEST_ASSERT_EQUAL(flashKLV.flash_pos(8), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(record4A.value, value);
}

void test_config()
{
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();

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
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);

    // read a config structure
    config_t configR {};
    err = flashKLV.read(&configR, sizeof(configR), CONFIG_KEY);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);

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
