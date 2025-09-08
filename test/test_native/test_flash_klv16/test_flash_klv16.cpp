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
    TEST_ASSERT_TRUE(FlashKLV::overwriteable(f0, v0));
    TEST_ASSERT_FALSE(FlashKLV::overwriteable(v0, f0));
    TEST_ASSERT_TRUE(FlashKLV::overwriteable(&f0, &v0, sizeof(f0)));
    TEST_ASSERT_FALSE(FlashKLV::overwriteable(&v0, &f0, sizeof(f0)));

    const uint8_t f1 = 0b11111111;
    const uint8_t v1 = 0b01010101;
    TEST_ASSERT_TRUE(FlashKLV::overwriteable(f1, v1));
    TEST_ASSERT_FALSE(FlashKLV::overwriteable(v1, f1));
    TEST_ASSERT_TRUE(FlashKLV::overwriteable(&f1, &v1, sizeof(f1)));
    TEST_ASSERT_FALSE(FlashKLV::overwriteable(&v1, &f1, sizeof(f1)));
}

void test_klv16()
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

    const record3_t record3A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record3A.key));
    const record4_t record4A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record4A.key));
    const record400_t record400A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record400A.key));
    const record7_t record7A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record7A.key));
    const record12_t record12A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record12A.key));
    const record23_t record23A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record23A.key));
}

void test_klv()
{
    flashMemory.fill(0xFF);
    FlashKLV flashKLV(&flashMemory[0], FlashKLV::SECTOR_SIZE*SECTOR_COUNT);

    record4_t recordA {};
    FlashKLV::klv_t klv {};
    int err {};
    uint32_t value {};

    TEST_ASSERT_EQUAL(8, recordA.length + sizeof(FlashKLV::kl16_t));
    klv = flashKLV.find(recordA.key);
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

    recordA.value = 0x7B536AFE;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    TEST_ASSERT_EQUAL(0x2B | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x81, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8)); // check byte after record still set to 0xFF

    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));

    TEST_ASSERT_EQUAL(false, flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(0x2B81, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(4, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(8, flashKLV.getRecordPositionIncrement(0));
    TEST_ASSERT_EQUAL_PTR(&flashMemory[4], flashKLV.getRecordValuePtr(0));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    value = *reinterpret_cast<uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // write the same value again, so should not be written and be in same position
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_NO_NEED_TO_WRITE, err);

    TEST_ASSERT_EQUAL(0x2B | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x81, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(0x2B81, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    value = *reinterpret_cast<uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // a value of 0x7B536A00 will allow the record to be overwritten
    recordA.value = 0x7B536A00;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_OVERWRITTEN, err);

    TEST_ASSERT_EQUAL(0x2B | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x81, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    value = *reinterpret_cast<uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // a value of 0xFFFFwill will require the record to be marked as deleted and a new record written
    recordA.value = 0x0C0B0AFE;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    // old first record, marked as deleted
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(0x2B | KL16_BIT, flashKLV.flashPeek(0)); // first has top bit cleared to mark as delete
    TEST_ASSERT_EQUAL(0x81, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(7));

    // new record
    TEST_ASSERT_EQUAL(0x2B | TOP_BITS, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(0x81, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(0x0B, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(0x0C, flashKLV.flashPeek(15));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(16)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(17));
    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(12), klv.valuePtr);
    value = *reinterpret_cast<uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(recordA.value, value);
}

void test_klv2()
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

    record400_t recordA {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(recordA.key));
    FlashKLV::klv_t klv {};
    int err {};
    uint32_t value {};

    TEST_ASSERT_EQUAL(8, recordA.length + sizeof(FlashKLV::klv_t::key) + sizeof(FlashKLV::klv_t::length));
    klv = flashKLV.find(recordA.key);
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

    recordA.value = 0x7B536AFE;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(0x0100, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(4, flashKLV.getRecordLength(0));

    TEST_ASSERT_EQUAL(0x01 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    value = *reinterpret_cast<uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // write the same value again, so should not be written and be in same position
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_NO_NEED_TO_WRITE, err);
    TEST_ASSERT_EQUAL(0x0100, flashKLV.getRecordKey(0));

    TEST_ASSERT_EQUAL(0x01 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(7));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    value = *reinterpret_cast<uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // a value of 0x7B536A00 will allow the record to be overwritten
    recordA.value = 0x7B536A00;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_OVERWRITTEN, err);

    TEST_ASSERT_EQUAL(0x01 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(7));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));

    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    value = *reinterpret_cast<uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(recordA.value, value);

    // a value of 0xFFFFwill will require the record to be marked as deleted and a new record written
    recordA.value = 0x0C0B0AFE;
    err = flashKLV.write(recordA.key, recordA.length, &recordA.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    // old first record, marked as deleted
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(0x01 | KL16_BIT, flashKLV.flashPeek(0)); // first byte set to zero to mark as deleted
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(7));

    // new record
    TEST_ASSERT_EQUAL(0x01 | TOP_BITS, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(0x0B, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(0x0C, flashKLV.flashPeek(15));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(16)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(17));
    klv = flashKLV.find(recordA.key);
    TEST_ASSERT_EQUAL(recordA.key, klv.key);
    TEST_ASSERT_EQUAL(recordA.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(12), klv.valuePtr);
    value = *reinterpret_cast<uint32_t*>(klv.valuePtr);
    TEST_ASSERT_EQUAL(recordA.value, value);
}

void test_multi_page_records()
{
    flashMemory.fill(0xFF);
    FlashKLV flashKLV(&flashMemory[0], FlashKLV::SECTOR_SIZE*SECTOR_COUNT);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));

    record12_t record12A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record12A.key));
    TEST_ASSERT_EQUAL(0x3571, record12A.key);
    TEST_ASSERT_EQUAL(16, record12A.length + sizeof(FlashKLV::klv_t::key) + sizeof(FlashKLV::klv_t::length));

    FlashKLV::klv_t klv {};
    int err {};

    klv = flashKLV.find(record12A.key);
    TEST_ASSERT_TRUE(klv.key == FlashKLV::NOT_FOUND);
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(0));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.getRecordLength(0));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));
    TEST_ASSERT_EQUAL(FlashKLV::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.valuePtr);

// Write a record
    record12A.value = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    err = flashKLV.write(record12A.key, record12A.length, &record12A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(0x3571, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(12, flashKLV.getRecordLength(0));

    TEST_ASSERT_EQUAL(0x35 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x71, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(5, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(8, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(10, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(15));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(16)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(16));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(16));

    klv = flashKLV.find(record12A.key);
    TEST_ASSERT_EQUAL(record12A.key, klv.key);
    TEST_ASSERT_EQUAL(record12A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    TEST_ASSERT_EQUAL(record12A.value[0], *klv.valuePtr);

// Overwrite record
    record12A.value = { 1, 0, 2, 4, 4, 6, 7, 0, 9, 2, 11, 12 };
    err = flashKLV.write(record12A.key, record12A.length, &record12A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_OVERWRITTEN, err);
    TEST_ASSERT_EQUAL(0x3571, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(12, flashKLV.getRecordLength(0));

    TEST_ASSERT_EQUAL(0x35 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x71, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(15));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(16)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(16));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(16));

    klv = flashKLV.find(record12A.key);
    TEST_ASSERT_EQUAL(record12A.key, klv.key);
    TEST_ASSERT_EQUAL(record12A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    TEST_ASSERT_EQUAL(record12A.value[0], *klv.valuePtr);

// Write new record
    record12A.value = { 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41 };
    err = flashKLV.write(record12A.key, record12A.length, &record12A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    // vestiges of old record
    TEST_ASSERT_EQUAL(0x35 | KL16_BIT, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x71, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(15));
    // new record
    TEST_ASSERT_EQUAL(0x35 | TOP_BITS, flashKLV.flashPeek(16));
    TEST_ASSERT_EQUAL(0x71, flashKLV.flashPeek(17));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(18));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(19));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(20));
    TEST_ASSERT_EQUAL(5, flashKLV.flashPeek(21));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(22));
    TEST_ASSERT_EQUAL(11, flashKLV.flashPeek(23));
    TEST_ASSERT_EQUAL(13, flashKLV.flashPeek(24));
    TEST_ASSERT_EQUAL(17, flashKLV.flashPeek(25));
    TEST_ASSERT_EQUAL(19, flashKLV.flashPeek(26));
    TEST_ASSERT_EQUAL(23, flashKLV.flashPeek(27));
    TEST_ASSERT_EQUAL(29, flashKLV.flashPeek(28));
    TEST_ASSERT_EQUAL(31, flashKLV.flashPeek(29));
    TEST_ASSERT_EQUAL(37, flashKLV.flashPeek(30));
    TEST_ASSERT_EQUAL(41, flashKLV.flashPeek(31));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(32)); // check byte after record still set to 0xFF

    TEST_ASSERT_EQUAL(0x3571, flashKLV.getRecordKey(16));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(12, flashKLV.getRecordLength(0));

    klv = flashKLV.find(record12A.key);
    TEST_ASSERT_EQUAL(record12A.key, klv.key);
    TEST_ASSERT_EQUAL(record12A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(20), klv.valuePtr);
    TEST_ASSERT_EQUAL(record12A.value[0], *klv.valuePtr);
}

void test_length_3()
{
    flashMemory.fill(0xFF);
    FlashKLV flashKLV(&flashMemory[0], FlashKLV::SECTOR_SIZE*SECTOR_COUNT);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));

    record3_t record3A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record3A.key));
    TEST_ASSERT_EQUAL(0x0329, record3A.key);
    TEST_ASSERT_EQUAL(7, record3A.length + sizeof(FlashKLV::klv_t::key) + sizeof(FlashKLV::klv_t::length));

    FlashKLV::klv_t klv {};
    int err {};

    klv = flashKLV.find(record3A.key);
    TEST_ASSERT_EQUAL(FlashKLV::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.valuePtr);

    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(1));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));

// Write a record
    record3A.value = { 1, 2, 3 };
    err = flashKLV.write(record3A.key, record3A.length, &record3A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_FALSE(flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(3, flashKLV.getRecordLength(0));

    TEST_ASSERT_EQUAL(0x03 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x29, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(6));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(7)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0x0329, flashKLV.getRecordKey(0));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(7));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(7));

    klv = flashKLV.find(record3A.key);
    TEST_ASSERT_EQUAL(record3A.key, klv.key);
    TEST_ASSERT_EQUAL(record3A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    TEST_ASSERT_EQUAL(record3A.value[0], *klv.valuePtr);

// Overwrite record
    record3A.value = { 1, 0, 2 };
    err = flashKLV.write(record3A.key, record3A.length, &record3A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_OVERWRITTEN, err);
    TEST_ASSERT_EQUAL(0x0329, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(3, flashKLV.getRecordLength(0));

    TEST_ASSERT_EQUAL(0x03 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x29, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(6));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(7)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(7));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(7));

    klv = flashKLV.find(record3A.key);
    TEST_ASSERT_EQUAL(record3A.key, klv.key);
    TEST_ASSERT_EQUAL(record3A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    TEST_ASSERT_EQUAL(record3A.value[0], *klv.valuePtr);

// Write new record
    record3A.value = { 3, 5, 7 };
    err = flashKLV.write(record3A.key, record3A.length, &record3A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(0x03 | TOP_BITS, flashKLV.flashPeek(7));

    // vestiges of old record
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(0x03 | KL16_BIT, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x29, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(6));
    // new record
    TEST_ASSERT_EQUAL(0x03 | TOP_BITS, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0x29, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(5, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(14)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0x0329, flashKLV.getRecordKey(7));
    TEST_ASSERT_EQUAL(3, flashKLV.getRecordLength(7));

    klv = flashKLV.find(record3A.key);
    TEST_ASSERT_EQUAL(record3A.key, klv.key);
    TEST_ASSERT_EQUAL(record3A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(11), klv.valuePtr);
    TEST_ASSERT_EQUAL(record3A.value[0], *klv.valuePtr);
}

void test_length_7()
{
    flashMemory.fill(0xFF);
    FlashKLV flashKLV(&flashMemory[0], FlashKLV::SECTOR_SIZE*SECTOR_COUNT);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));

    record7_t record7A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record7A.key));
    TEST_ASSERT_EQUAL(0x0717, record7A.key);
    TEST_ASSERT_EQUAL(11, record7A.length + sizeof(FlashKLV::klv_t::key) + sizeof(FlashKLV::klv_t::length));

    FlashKLV::klv_t klv {};
    int err {};

    klv = flashKLV.find(record7A.key);
    TEST_ASSERT_TRUE(klv.key == FlashKLV::NOT_FOUND);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(0));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));
    TEST_ASSERT_EQUAL(FlashKLV::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.valuePtr);

// Write a record
    record7A.value = { 1, 2, 3, 4, 5, 6, 7 };
    err = flashKLV.write(record7A.key, record7A.length, &record7A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    TEST_ASSERT_EQUAL(0x07 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x17, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(5, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(11)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0x0717, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(11));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(11));
    TEST_ASSERT_EQUAL(7, flashKLV.getRecordLength(0));
    klv = flashKLV.find(record7A.key);
    TEST_ASSERT_EQUAL(record7A.key, klv.key);
    TEST_ASSERT_EQUAL(record7A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    TEST_ASSERT_EQUAL(record7A.value[0], *klv.valuePtr);

// Overwrite record
    record7A.value = { 1, 0, 2, 4, 4, 6, 7 };
    err = flashKLV.write(record7A.key, record7A.length, &record7A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_OVERWRITTEN, err);

    TEST_ASSERT_EQUAL(0x07 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x17, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(11)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0x0717, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(11));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(11));
    TEST_ASSERT_EQUAL(7, flashKLV.getRecordLength(0));
    klv = flashKLV.find(record7A.key);
    TEST_ASSERT_EQUAL(record7A.key, klv.key);
    TEST_ASSERT_EQUAL(record7A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    TEST_ASSERT_EQUAL(record7A.value[0], *klv.valuePtr);

// Write new record
    record7A.value = { 3, 5, 7, 11, 13, 17, 19 };
    err = flashKLV.write(record7A.key, record7A.length, &record7A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    // vestiges of old record
    TEST_ASSERT_EQUAL(0x07 | KL16_BIT, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x17, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(10));
    // new record
    TEST_ASSERT_EQUAL(0x07 | TOP_BITS, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(0x17, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(15));
    TEST_ASSERT_EQUAL(5, flashKLV.flashPeek(16));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(17));
    TEST_ASSERT_EQUAL(11, flashKLV.flashPeek(18));
    TEST_ASSERT_EQUAL(13, flashKLV.flashPeek(19));
    TEST_ASSERT_EQUAL(17, flashKLV.flashPeek(20));
    TEST_ASSERT_EQUAL(19, flashKLV.flashPeek(21));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(22)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0x0717, flashKLV.getRecordKey(11));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(0) & TOP_BIT);
    TEST_ASSERT_EQUAL(7, flashKLV.getRecordLength(11));
    klv = flashKLV.find(record7A.key);
    TEST_ASSERT_EQUAL(record7A.key, klv.key);
    TEST_ASSERT_EQUAL(record7A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(15), klv.valuePtr);
    TEST_ASSERT_EQUAL(record7A.value[0], *klv.valuePtr);
}

void test_length_23()
{
    flashMemory.fill(0xFF);
    FlashKLV flashKLV(&flashMemory[0], FlashKLV::SECTOR_SIZE*SECTOR_COUNT);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));

    record23_t record23A {};
    TEST_ASSERT_EQUAL(true, FlashKLV::keyOK(record23A.key));
    TEST_ASSERT_EQUAL(0x2361, record23A.key);
    TEST_ASSERT_EQUAL(27, record23A.length + sizeof(FlashKLV::klv_t::key) + sizeof(FlashKLV::klv_t::length));

    FlashKLV::klv_t klv {};
    int err {};

    klv = flashKLV.find(record23A.key);
    TEST_ASSERT_TRUE(klv.key == FlashKLV::NOT_FOUND);
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(0));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(0xFFFF, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(8));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(8));
    TEST_ASSERT_EQUAL(FlashKLV::NOT_FOUND, klv.key);
    TEST_ASSERT_EQUAL(0, klv.length);
    TEST_ASSERT_EQUAL(nullptr, klv.valuePtr);

// Write a record
    record23A.value = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
    err = flashKLV.write(record23A.key, record23A.length, &record23A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    TEST_ASSERT_EQUAL(0x23 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x61, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(23, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(5, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(8, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(10, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(15));
    TEST_ASSERT_EQUAL(13, flashKLV.flashPeek(16));
    TEST_ASSERT_EQUAL(14, flashKLV.flashPeek(17));
    TEST_ASSERT_EQUAL(15, flashKLV.flashPeek(18));
    TEST_ASSERT_EQUAL(16, flashKLV.flashPeek(19));
    TEST_ASSERT_EQUAL(17, flashKLV.flashPeek(20));
    TEST_ASSERT_EQUAL(18, flashKLV.flashPeek(21));
    TEST_ASSERT_EQUAL(19, flashKLV.flashPeek(22));
    TEST_ASSERT_EQUAL(20, flashKLV.flashPeek(23));
    TEST_ASSERT_EQUAL(21, flashKLV.flashPeek(24));
    TEST_ASSERT_EQUAL(22, flashKLV.flashPeek(25));
    TEST_ASSERT_EQUAL(23, flashKLV.flashPeek(26));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(27)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(28));
    TEST_ASSERT_EQUAL(0x2361, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(27));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(27));
    TEST_ASSERT_EQUAL(23, flashKLV.getRecordLength(0));
    klv = flashKLV.find(record23A.key);
    TEST_ASSERT_EQUAL(record23A.key, klv.key);
    TEST_ASSERT_EQUAL(record23A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    TEST_ASSERT_EQUAL(record23A.value[0], *klv.valuePtr);

// Overwrite record
    record23A.value = { 1, 0, 2, 4, 4, 6, 7, 0, 9, 2, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
    err = flashKLV.write(record23A.key, record23A.length, &record23A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK_OVERWRITTEN, err);

    TEST_ASSERT_EQUAL(0x23 | TOP_BITS, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x61, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(23, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(15));
    TEST_ASSERT_EQUAL(21, flashKLV.flashPeek(24));
    TEST_ASSERT_EQUAL(22, flashKLV.flashPeek(25));
    TEST_ASSERT_EQUAL(23, flashKLV.flashPeek(26));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(27)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(28));
    TEST_ASSERT_EQUAL(0x2361, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(FlashKLV::RECORD_KEY_EMPTY, flashKLV.getRecordKey(27));
    TEST_ASSERT_TRUE(flashKLV.isRecordEmpty(27));
    TEST_ASSERT_EQUAL(23, flashKLV.getRecordLength(0));
    klv = flashKLV.find(record23A.key);
    TEST_ASSERT_EQUAL(record23A.key, klv.key);
    TEST_ASSERT_EQUAL(record23A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(4), klv.valuePtr);
    TEST_ASSERT_EQUAL(record23A.value[0], *klv.valuePtr);

// Write new record
    record23A.value = { 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 52, 63, 74, 85, 96, 107, 118, 139, 140, 151, 162 };
    err = flashKLV.write(record23A.key, record23A.length, &record23A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(0) & TOP_BIT);

    // vestiges of old record
    TEST_ASSERT_EQUAL(0x23 | KL16_BIT, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x61, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(23, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(4, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(6, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(9, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(2, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(11, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(12, flashKLV.flashPeek(15));
    TEST_ASSERT_EQUAL(22, flashKLV.flashPeek(25));
    TEST_ASSERT_EQUAL(23, flashKLV.flashPeek(26));
    // new record
    TEST_ASSERT_EQUAL(0x23 | TOP_BITS, flashKLV.flashPeek(27));
    TEST_ASSERT_EQUAL(0x61, flashKLV.flashPeek(28));
    TEST_ASSERT_EQUAL(23, flashKLV.flashPeek(29));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(30));
    TEST_ASSERT_EQUAL(3, flashKLV.flashPeek(31));
    TEST_ASSERT_EQUAL(5, flashKLV.flashPeek(32));
    TEST_ASSERT_EQUAL(7, flashKLV.flashPeek(33));
    TEST_ASSERT_EQUAL(11, flashKLV.flashPeek(34));
    TEST_ASSERT_EQUAL(13, flashKLV.flashPeek(35));
    TEST_ASSERT_EQUAL(17, flashKLV.flashPeek(36));
    TEST_ASSERT_EQUAL(19, flashKLV.flashPeek(37));
    TEST_ASSERT_EQUAL(23, flashKLV.flashPeek(38));
    TEST_ASSERT_EQUAL(29, flashKLV.flashPeek(39));
    TEST_ASSERT_EQUAL(31, flashKLV.flashPeek(40));
    TEST_ASSERT_EQUAL(37, flashKLV.flashPeek(41));
    TEST_ASSERT_EQUAL(41, flashKLV.flashPeek(42));
    TEST_ASSERT_EQUAL(52, flashKLV.flashPeek(43));
    TEST_ASSERT_EQUAL(63, flashKLV.flashPeek(44));
    TEST_ASSERT_EQUAL(74, flashKLV.flashPeek(45));
    TEST_ASSERT_EQUAL(85, flashKLV.flashPeek(46));
    TEST_ASSERT_EQUAL(96, flashKLV.flashPeek(47));
    TEST_ASSERT_EQUAL(107, flashKLV.flashPeek(48));
    TEST_ASSERT_EQUAL(118, flashKLV.flashPeek(49));
    TEST_ASSERT_EQUAL(139, flashKLV.flashPeek(50));
    TEST_ASSERT_EQUAL(140, flashKLV.flashPeek(51));
    TEST_ASSERT_EQUAL(151, flashKLV.flashPeek(52));
    TEST_ASSERT_EQUAL(162, flashKLV.flashPeek(53));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(54)); // check byte after record still set to 0xFF
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(55));

    TEST_ASSERT_EQUAL(0x2361, flashKLV.getRecordKey(27));
    TEST_ASSERT_EQUAL(23, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(23, flashKLV.getRecordLength(27));
    klv = flashKLV.find(record23A.key);
    TEST_ASSERT_EQUAL(record23A.key, klv.key);
    TEST_ASSERT_EQUAL(record23A.length, klv.length);
    TEST_ASSERT_EQUAL(flashKLV.flashPos(31), klv.valuePtr);
    TEST_ASSERT_EQUAL(record23A.value[0], *klv.valuePtr);
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
