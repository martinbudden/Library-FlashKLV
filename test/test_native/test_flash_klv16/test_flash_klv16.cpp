#include <FlashKlv.h>
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

enum : uint8_t { TOP_BIT = 0x80, KL16_BIT = 0x40, TOP_BITS = TOP_BIT|KL16_BIT };

struct record3_t {
    enum { LENGTH = 3 };
    uint16_t key = 0x0329;
    uint16_t length = LENGTH;
    std::array<uint8_t, LENGTH> value {};
};

struct record4_t {
    uint16_t key = 0x2B81;
    uint16_t length = sizeof(int32_t);
    int32_t value = 0;
};

struct record400_t {
    uint16_t key = 0x0100;
    uint16_t length = sizeof(int32_t);
    int32_t value = 0;
};

struct record7_t {
    enum { LENGTH = 7 };
    uint16_t key = 0x0717;
    uint16_t length = LENGTH;
    std::array<uint8_t, LENGTH> value {};
};

struct record12_t {
    enum { LENGTH = 12 };
    uint16_t key = 0x3571;
    uint16_t length = LENGTH;
    std::array<uint8_t, LENGTH> value {};
};

struct record23_t {
    enum { LENGTH = 23 };
    uint16_t key = 0x2361;
    uint16_t length = LENGTH;
    std::array<uint8_t, LENGTH> value {};
};

void test_flash_overwriteable()
{
    const uint8_t f0 = 0b11111111;
    const uint8_t v0 = 0b00000000;
    TEST_ASSERT_TRUE(FlashKlv::overwriteable(f0, v0));
    TEST_ASSERT_FALSE(FlashKlv::overwriteable(v0, f0));
    TEST_ASSERT_TRUE(FlashKlv::overwriteable(&f0, &v0, sizeof(f0)));
    TEST_ASSERT_FALSE(FlashKlv::overwriteable(&v0, &f0, sizeof(f0)));

    const uint8_t f1 = 0b11111111;
    const uint8_t v1 = 0b01010101;
    TEST_ASSERT_TRUE(FlashKlv::overwriteable(f1, v1));
    TEST_ASSERT_FALSE(FlashKlv::overwriteable(v1, f1));
    TEST_ASSERT_TRUE(FlashKlv::overwriteable(&f1, &v1, sizeof(f1)));
    TEST_ASSERT_FALSE(FlashKlv::overwriteable(&v1, &f1, sizeof(f1)));
}

void test_klv16()
{
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(FlashKlv::SECTOR_SIZE*SECTOR_COUNT - 1));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8));

    const record3_t record3A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record3A.key));
    const record4_t record4A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record4A.key));
    const record400_t record400A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record400A.key));
    const record7_t record7A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record7A.key));
    const record12_t record12A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record12A.key));
    const record23_t record23A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record23A.key));
}

void test_klv()
{
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(FlashKlv::SECTOR_SIZE*SECTOR_COUNT - 1));

    record4_t recordA {};
    FlashKlv::klv_t klv {};
    int err {};
    uint32_t value {};

    TEST_ASSERT_EQUAL(8, recordA.length + sizeof(FlashKlv::kl16_t));
    klv = flashKLV.find(recordA.key);
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

    recordA.value = 0x7B536AFE;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);

    TEST_ASSERT_EQUAL(0x2B | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x81, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8)); // check byte after record still set to 0xFF

    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));

    TEST_ASSERT_EQUAL(false, flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(0x2B81, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(4, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(8, flashKLV.get_record_position_increment(0));
    TEST_ASSERT_EQUAL_PTR(&flashMemory[4], flashKLV.get_record_value_ptr(0));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // write the same value again, so should not be written and be in same position
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_NO_NEED_TO_WRITE, err);

    TEST_ASSERT_EQUAL(0x2B | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x81, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(0x2B81, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // a value of 0x7B536A00 will allow the record to be overwritten
    recordA.value = 0x7B536A00;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_OVERWRITTEN, err);

    TEST_ASSERT_EQUAL(0x2B | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x81, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // a value of 0xFFFFwill will require the record to be marked as deleted and a new record written
    recordA.value = 0x0C0B0AFE;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);

    // old first record, marked as deleted
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(0x2B | KL16_BIT, flashKLV.flash_peek(0)); // first has top bit cleared to mark as delete
    TEST_ASSERT_EQUAL(0x81, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(7));

    // new record
    TEST_ASSERT_EQUAL(0x2B | TOP_BITS, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(0x81, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(0x0B, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(0x0C, flashKLV.flash_peek(15));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(16)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(17));
    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(12), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(recordA.value, value);
}

void test_klv2()
{
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(FlashKlv::SECTOR_SIZE*SECTOR_COUNT - 1));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8));

    record400_t recordA {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(recordA.key));
    FlashKlv::klv_t klv {};
    int err {};
    uint32_t value {};

    TEST_ASSERT_EQUAL(8, recordA.length + sizeof(FlashKlv::klv_t::key) + sizeof(FlashKlv::klv_t::length));
    klv = flashKLV.find(recordA.key);
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

    recordA.value = 0x7B536AFE;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(0x0100, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(4, flashKLV.get_record_length(0));

    TEST_ASSERT_EQUAL(0x01 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // write the same value again, so should not be written and be in same position
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_NO_NEED_TO_WRITE, err);
    TEST_ASSERT_EQUAL(0x0100, flashKLV.get_record_key(0));

    TEST_ASSERT_EQUAL(0x01 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(7));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // a value of 0x7B536A00 will allow the record to be overwritten
    recordA.value = 0x7B536A00;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_OVERWRITTEN, err);

    TEST_ASSERT_EQUAL(0x01 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(7));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // a value of 0xFFFFwill will require the record to be marked as deleted and a new record written
    recordA.value = 0x0C0B0AFE;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);

    // old first record, marked as deleted
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(0x01 | KL16_BIT, flashKLV.flash_peek(0)); // first byte set to zero to mark as deleted
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(7));

    // new record
    TEST_ASSERT_EQUAL(0x01 | TOP_BITS, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(0x0B, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(0x0C, flashKLV.flash_peek(15));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(16)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(17));
    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(12), klv.value_ptr);
    value = *reinterpret_cast<const uint32_t*>(klv.value_ptr);
    TEST_ASSERT_EQUAL(recordA.value, value);
}

void test_multi_page_records()
{
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(FlashKlv::SECTOR_SIZE*SECTOR_COUNT - 1));

    record12_t record12A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record12A.key));
    TEST_ASSERT_EQUAL(0x3571, record12A.key);
    TEST_ASSERT_EQUAL(16, record12A.length + sizeof(FlashKlv::klv_t::key) + sizeof(FlashKlv::klv_t::length));

    FlashKlv::klv_t klv {};
    int err {};

    klv = flashKLV.find(record12A.key);
    TEST_ASSERT_TRUE(klv.key == FlashKlv::NOT_FOUND);
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(0));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.get_record_length(0));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));
    TEST_ASSERT_EQUAL(FlashKlv::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.value_ptr);

// Write a record
    record12A.value = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    err = flashKLV.write(record12A.key, record12A.length, &record12A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(0x3571, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(12, flashKLV.get_record_length(0));

    TEST_ASSERT_EQUAL(0x35 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x71, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(5, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(8, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(10, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(15));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(16)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(16));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(16));

    klv = flashKLV.find(record12A.key);
    TEST_ASSERT_EQUAL(record12A.key, klv.key);
    TEST_ASSERT_EQUAL(record12A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    TEST_ASSERT_EQUAL(record12A.value[0], *klv.value_ptr);

// Overwrite record
    record12A.value = { 1, 0, 2, 4, 4, 6, 7, 0, 9, 2, 11, 12 };
    err = flashKLV.write(record12A.key, record12A.length, &record12A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_OVERWRITTEN, err);
    TEST_ASSERT_EQUAL(0x3571, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(12, flashKLV.get_record_length(0));

    TEST_ASSERT_EQUAL(0x35 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x71, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(15));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(16)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(16));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(16));

    klv = flashKLV.find(record12A.key);
    TEST_ASSERT_EQUAL(record12A.key, klv.key);
    TEST_ASSERT_EQUAL(record12A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    TEST_ASSERT_EQUAL(record12A.value[0], *klv.value_ptr);

// Write new record
    record12A.value = { 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41 };
    err = flashKLV.write(record12A.key, record12A.length, &record12A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);

    // vestiges of old record
    TEST_ASSERT_EQUAL(0x35 | KL16_BIT, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x71, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(15));
    // new record
    TEST_ASSERT_EQUAL(0x35 | TOP_BITS, flashKLV.flash_peek(16));
    TEST_ASSERT_EQUAL(0x71, flashKLV.flash_peek(17));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(18));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(19));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(20));
    TEST_ASSERT_EQUAL(5, flashKLV.flash_peek(21));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(22));
    TEST_ASSERT_EQUAL(11, flashKLV.flash_peek(23));
    TEST_ASSERT_EQUAL(13, flashKLV.flash_peek(24));
    TEST_ASSERT_EQUAL(17, flashKLV.flash_peek(25));
    TEST_ASSERT_EQUAL(19, flashKLV.flash_peek(26));
    TEST_ASSERT_EQUAL(23, flashKLV.flash_peek(27));
    TEST_ASSERT_EQUAL(29, flashKLV.flash_peek(28));
    TEST_ASSERT_EQUAL(31, flashKLV.flash_peek(29));
    TEST_ASSERT_EQUAL(37, flashKLV.flash_peek(30));
    TEST_ASSERT_EQUAL(41, flashKLV.flash_peek(31));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(32)); // check byte after record still set to 0xFF

    TEST_ASSERT_EQUAL(0x3571, flashKLV.get_record_key(16));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(12, flashKLV.get_record_length(0));

    klv = flashKLV.find(record12A.key);
    TEST_ASSERT_EQUAL(record12A.key, klv.key);
    TEST_ASSERT_EQUAL(record12A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(20), klv.value_ptr);
    TEST_ASSERT_EQUAL(record12A.value[0], *klv.value_ptr);
}

void test_length_3()
{
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(FlashKlv::SECTOR_SIZE*SECTOR_COUNT - 1));

    record3_t record3A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record3A.key));
    TEST_ASSERT_EQUAL(0x0329, record3A.key);
    TEST_ASSERT_EQUAL(7, record3A.length + sizeof(FlashKlv::klv_t::key) + sizeof(FlashKlv::klv_t::length));

    FlashKlv::klv_t klv {};
    int err {};

    klv = flashKLV.find(record3A.key);
    TEST_ASSERT_EQUAL(FlashKlv::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.value_ptr);

    TEST_ASSERT_TRUE(flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(1));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));

// Write a record
    record3A.value = { 1, 2, 3 };
    err = flashKLV.write(record3A.key, record3A.length, &record3A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_FALSE(flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(3, flashKLV.get_record_length(0));

    TEST_ASSERT_EQUAL(0x03 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x29, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(6));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0x0329, flashKLV.get_record_key(0));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(7));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(7));

    klv = flashKLV.find(record3A.key);
    TEST_ASSERT_EQUAL(record3A.key, klv.key);
    TEST_ASSERT_EQUAL(record3A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    TEST_ASSERT_EQUAL(record3A.value[0], *klv.value_ptr);

// Overwrite record
    record3A.value = { 1, 0, 2 };
    err = flashKLV.write(record3A.key, record3A.length, &record3A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_OVERWRITTEN, err);
    TEST_ASSERT_EQUAL(0x0329, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(3, flashKLV.get_record_length(0));

    TEST_ASSERT_EQUAL(0x03 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x29, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(6));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(7)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(7));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(7));

    klv = flashKLV.find(record3A.key);
    TEST_ASSERT_EQUAL(record3A.key, klv.key);
    TEST_ASSERT_EQUAL(record3A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    TEST_ASSERT_EQUAL(record3A.value[0], *klv.value_ptr);

// Write new record
    record3A.value = { 3, 5, 7 };
    err = flashKLV.write(record3A.key, record3A.length, &record3A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(0x03 | TOP_BITS, flashKLV.flash_peek(7));

    // vestiges of old record
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(0x03 | KL16_BIT, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x29, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(6));
    // new record
    TEST_ASSERT_EQUAL(0x03 | TOP_BITS, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0x29, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(5, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(14)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0x0329, flashKLV.get_record_key(7));
    TEST_ASSERT_EQUAL(3, flashKLV.get_record_length(7));

    klv = flashKLV.find(record3A.key);
    TEST_ASSERT_EQUAL(record3A.key, klv.key);
    TEST_ASSERT_EQUAL(record3A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(11), klv.value_ptr);
    TEST_ASSERT_EQUAL(record3A.value[0], *klv.value_ptr);
}

void test_length_7()
{
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(FlashKlv::SECTOR_SIZE*SECTOR_COUNT - 1));

    record7_t record7A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record7A.key));
    TEST_ASSERT_EQUAL(0x0717, record7A.key);
    TEST_ASSERT_EQUAL(11, record7A.length + sizeof(FlashKlv::klv_t::key) + sizeof(FlashKlv::klv_t::length));

    FlashKlv::klv_t klv {};
    int err {};

    klv = flashKLV.find(record7A.key);
    TEST_ASSERT_TRUE(klv.key == FlashKlv::NOT_FOUND);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(0));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));
    TEST_ASSERT_EQUAL(FlashKlv::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.value_ptr);

// Write a record
    record7A.value = { 1, 2, 3, 4, 5, 6, 7 };
    err = flashKLV.write(record7A.key, record7A.length, &record7A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);

    TEST_ASSERT_EQUAL(0x07 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x17, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(5, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(11)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0x0717, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(11));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(11));
    TEST_ASSERT_EQUAL(7, flashKLV.get_record_length(0));
    klv = flashKLV.find(record7A.key);
    TEST_ASSERT_EQUAL(record7A.key, klv.key);
    TEST_ASSERT_EQUAL(record7A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    TEST_ASSERT_EQUAL(record7A.value[0], *klv.value_ptr);

// Overwrite record
    record7A.value = { 1, 0, 2, 4, 4, 6, 7 };
    err = flashKLV.write(record7A.key, record7A.length, &record7A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_OVERWRITTEN, err);

    TEST_ASSERT_EQUAL(0x07 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x17, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(11)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0x0717, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(11));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(11));
    TEST_ASSERT_EQUAL(7, flashKLV.get_record_length(0));
    klv = flashKLV.find(record7A.key);
    TEST_ASSERT_EQUAL(record7A.key, klv.key);
    TEST_ASSERT_EQUAL(record7A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    TEST_ASSERT_EQUAL(record7A.value[0], *klv.value_ptr);

// Write new record
    record7A.value = { 3, 5, 7, 11, 13, 17, 19 };
    err = flashKLV.write(record7A.key, record7A.length, &record7A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);

    // vestiges of old record
    TEST_ASSERT_EQUAL(0x07 | KL16_BIT, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x17, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(10));
    // new record
    TEST_ASSERT_EQUAL(0x07 | TOP_BITS, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(0x17, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(15));
    TEST_ASSERT_EQUAL(5, flashKLV.flash_peek(16));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(17));
    TEST_ASSERT_EQUAL(11, flashKLV.flash_peek(18));
    TEST_ASSERT_EQUAL(13, flashKLV.flash_peek(19));
    TEST_ASSERT_EQUAL(17, flashKLV.flash_peek(20));
    TEST_ASSERT_EQUAL(19, flashKLV.flash_peek(21));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(22)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0x0717, flashKLV.get_record_key(11));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(7, flashKLV.get_record_length(11));
    klv = flashKLV.find(record7A.key);
    TEST_ASSERT_EQUAL(record7A.key, klv.key);
    TEST_ASSERT_EQUAL(record7A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(15), klv.value_ptr);
    TEST_ASSERT_EQUAL(record7A.value[0], *klv.value_ptr);
}

void test_length_23()
{
    FlashKlv flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.erase_current_bank();
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(FlashKlv::SECTOR_SIZE*SECTOR_COUNT - 1));

    record23_t record23A {};
    TEST_ASSERT_EQUAL(true, FlashKlv::key_ok(record23A.key));
    TEST_ASSERT_EQUAL(0x2361, record23A.key);
    TEST_ASSERT_EQUAL(27, record23A.length + sizeof(FlashKlv::klv_t::key) + sizeof(FlashKlv::klv_t::length));

    FlashKlv::klv_t klv {};
    int err {};

    klv = flashKLV.find(record23A.key);
    TEST_ASSERT_TRUE(klv.key == FlashKlv::NOT_FOUND);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(0));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(8));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(8));
    TEST_ASSERT_EQUAL(FlashKlv::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.value_ptr);

// Write a record
    record23A.value = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
    err = flashKLV.write(record23A.key, record23A.length, &record23A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);

    TEST_ASSERT_EQUAL(0x23 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x61, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(23, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(5, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(8, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(10, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(15));
    TEST_ASSERT_EQUAL(13, flashKLV.flash_peek(16));
    TEST_ASSERT_EQUAL(14, flashKLV.flash_peek(17));
    TEST_ASSERT_EQUAL(15, flashKLV.flash_peek(18));
    TEST_ASSERT_EQUAL(16, flashKLV.flash_peek(19));
    TEST_ASSERT_EQUAL(17, flashKLV.flash_peek(20));
    TEST_ASSERT_EQUAL(18, flashKLV.flash_peek(21));
    TEST_ASSERT_EQUAL(19, flashKLV.flash_peek(22));
    TEST_ASSERT_EQUAL(20, flashKLV.flash_peek(23));
    TEST_ASSERT_EQUAL(21, flashKLV.flash_peek(24));
    TEST_ASSERT_EQUAL(22, flashKLV.flash_peek(25));
    TEST_ASSERT_EQUAL(23, flashKLV.flash_peek(26));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(27)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(28));
    TEST_ASSERT_EQUAL(0x2361, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(27));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(27));
    TEST_ASSERT_EQUAL(23, flashKLV.get_record_length(0));
    klv = flashKLV.find(record23A.key);
    TEST_ASSERT_EQUAL(record23A.key, klv.key);
    TEST_ASSERT_EQUAL(record23A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    TEST_ASSERT_EQUAL(record23A.value[0], *klv.value_ptr);

// Overwrite record
    record23A.value = { 1, 0, 2, 4, 4, 6, 7, 0, 9, 2, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
    err = flashKLV.write(record23A.key, record23A.length, &record23A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK_OVERWRITTEN, err);

    TEST_ASSERT_EQUAL(0x23 | TOP_BITS, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x61, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(23, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(15));
    TEST_ASSERT_EQUAL(21, flashKLV.flash_peek(24));
    TEST_ASSERT_EQUAL(22, flashKLV.flash_peek(25));
    TEST_ASSERT_EQUAL(23, flashKLV.flash_peek(26));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(27)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(28));
    TEST_ASSERT_EQUAL(0x2361, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(FlashKlv::RECORD_KEY_EMPTY, flashKLV.get_record_key(27));
    TEST_ASSERT_TRUE(flashKLV.is_record_empty(27));
    TEST_ASSERT_EQUAL(23, flashKLV.get_record_length(0));
    klv = flashKLV.find(record23A.key);
    TEST_ASSERT_EQUAL(record23A.key, klv.key);
    TEST_ASSERT_EQUAL(record23A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(4), klv.value_ptr);
    TEST_ASSERT_EQUAL(record23A.value[0], *klv.value_ptr);

// Write new record
    record23A.value = { 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 52, 63, 74, 85, 96, 107, 118, 139, 140, 151, 162 };
    err = flashKLV.write(record23A.key, record23A.length, &record23A.value);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(0) & TOP_BIT);

    // vestiges of old record
    TEST_ASSERT_EQUAL(0x23 | KL16_BIT, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x61, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(23, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(2, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flash_peek(15));
    TEST_ASSERT_EQUAL(22, flashKLV.flash_peek(25));
    TEST_ASSERT_EQUAL(23, flashKLV.flash_peek(26));
    // new record
    TEST_ASSERT_EQUAL(0x23 | TOP_BITS, flashKLV.flash_peek(27));
    TEST_ASSERT_EQUAL(0x61, flashKLV.flash_peek(28));
    TEST_ASSERT_EQUAL(23, flashKLV.flash_peek(29));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(30));
    TEST_ASSERT_EQUAL(3, flashKLV.flash_peek(31));
    TEST_ASSERT_EQUAL(5, flashKLV.flash_peek(32));
    TEST_ASSERT_EQUAL(7, flashKLV.flash_peek(33));
    TEST_ASSERT_EQUAL(11, flashKLV.flash_peek(34));
    TEST_ASSERT_EQUAL(13, flashKLV.flash_peek(35));
    TEST_ASSERT_EQUAL(17, flashKLV.flash_peek(36));
    TEST_ASSERT_EQUAL(19, flashKLV.flash_peek(37));
    TEST_ASSERT_EQUAL(23, flashKLV.flash_peek(38));
    TEST_ASSERT_EQUAL(29, flashKLV.flash_peek(39));
    TEST_ASSERT_EQUAL(31, flashKLV.flash_peek(40));
    TEST_ASSERT_EQUAL(37, flashKLV.flash_peek(41));
    TEST_ASSERT_EQUAL(41, flashKLV.flash_peek(42));
    TEST_ASSERT_EQUAL(52, flashKLV.flash_peek(43));
    TEST_ASSERT_EQUAL(63, flashKLV.flash_peek(44));
    TEST_ASSERT_EQUAL(74, flashKLV.flash_peek(45));
    TEST_ASSERT_EQUAL(85, flashKLV.flash_peek(46));
    TEST_ASSERT_EQUAL(96, flashKLV.flash_peek(47));
    TEST_ASSERT_EQUAL(107, flashKLV.flash_peek(48));
    TEST_ASSERT_EQUAL(118, flashKLV.flash_peek(49));
    TEST_ASSERT_EQUAL(139, flashKLV.flash_peek(50));
    TEST_ASSERT_EQUAL(140, flashKLV.flash_peek(51));
    TEST_ASSERT_EQUAL(151, flashKLV.flash_peek(52));
    TEST_ASSERT_EQUAL(162, flashKLV.flash_peek(53));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(54)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(55));

    TEST_ASSERT_EQUAL(0x2361, flashKLV.get_record_key(27));
    TEST_ASSERT_EQUAL(23, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(23, flashKLV.get_record_length(27));
    klv = flashKLV.find(record23A.key);
    TEST_ASSERT_EQUAL(record23A.key, klv.key);
    TEST_ASSERT_EQUAL(record23A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flash_pos(31), klv.value_ptr);
    TEST_ASSERT_EQUAL(record23A.value[0], *klv.value_ptr);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-reinterpret-cast,hicpp-signed-bitwise,readability-identifier-length,readability-magic-numbers,altera-struct-pack-align)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_flash_overwriteable);
    RUN_TEST(test_klv16);
    RUN_TEST(test_klv);
    RUN_TEST(test_klv2);
    RUN_TEST(test_multi_page_records);
    RUN_TEST(test_length_3);
    RUN_TEST(test_length_7);
    RUN_TEST(test_length_23);

    UNITY_END();
}
