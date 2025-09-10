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
enum { BANKB_OFFSET = FlashKLV::SECTOR_SIZE*SECTOR_COUNT };
static std::array<uint8_t, FlashKLV::SECTOR_SIZE*SECTOR_COUNT> flashMemory;
static std::array<uint8_t, FlashKLV::SECTOR_SIZE*SECTOR_COUNT*2> flashMemory2Banks;

// for FRAMEWORK_TEST, PAGE_SIZE = 8

enum : uint8_t { TOP_BIT = 0x80, WORD_BIT = 0x40, TOP_BITS = TOP_BIT|WORD_BIT };

struct record4A_t {
    uint16_t key = 0x14;
    uint16_t length = sizeof(int32_t);
    int32_t value = 0;
};

struct record4B_t {
    uint16_t key = 0x24;
    uint16_t length = sizeof(int32_t);
    int32_t value = 0;
};

void test_one_bank()
{
    flashMemory.fill(0);
    FlashKLV flashKLV(&flashMemory[0], SECTOR_COUNT);
    flashKLV.eraseCurrentBank();

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(FlashKLV::SECTOR_SIZE*SECTOR_COUNT - 1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8));

    TEST_ASSERT_EQUAL(flashKLV.memorySize(), flashKLV.bytesFree());
}

void test_two_banks()
{
    flashMemory2Banks.fill(0);
    FlashKLV flashKLV(&flashMemory2Banks[0], SECTOR_COUNT, FlashKLV::TWO_BANKS);
    TEST_ASSERT_EQUAL(flashKLV.getCurrentBankMemoryPtr() + BANKB_OFFSET, flashKLV.getOtherBankMemoryPtr());
    TEST_ASSERT_EQUAL(false, flashKLV.isCurrentBankErased());
    TEST_ASSERT_EQUAL(false, flashKLV.isOtherBankErased());

    flashKLV.eraseCurrentBank();
    TEST_ASSERT_EQUAL(true, flashKLV.isCurrentBankErased());
    TEST_ASSERT_EQUAL(flashKLV.memorySize(), flashKLV.bytesFree());

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(FlashKLV::SECTOR_SIZE*SECTOR_COUNT - 1));

    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(BANKB_OFFSET));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(BANKB_OFFSET + 8));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(2*FlashKLV::SECTOR_SIZE*SECTOR_COUNT - 1));
}

void test_copy_1()
{
    flashMemory2Banks.fill(0);
    FlashKLV flashKLV(&flashMemory2Banks[0], SECTOR_COUNT, FlashKLV::TWO_BANKS);
    TEST_ASSERT_EQUAL(flashKLV.getCurrentBankMemoryPtr() + BANKB_OFFSET, flashKLV.getOtherBankMemoryPtr());
    flashKLV.eraseCurrentBank();

    int err {};
    record4A_t record4A {};

    record4A.value = 0x7B536AFE;
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(flashKLV.memorySize() - 6, flashKLV.bytesFree());
    TEST_ASSERT_EQUAL(false, flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(0x14, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(4, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(6, flashKLV.getRecordPositionIncrement(0));
    TEST_ASSERT_EQUAL_PTR(&flashMemory2Banks[2], flashKLV.getRecordValuePtr(0));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(5));

    TEST_ASSERT_EQUAL(false, flashKLV.isOtherBankErased());
    flashKLV.eraseOtherBank();
    TEST_ASSERT_EQUAL(true, flashKLV.isOtherBankErased());
    TEST_ASSERT_EQUAL(true, flashKLV.isOtherBankErased());
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(7));

    TEST_ASSERT_EQUAL(512*2, BANKB_OFFSET);
    err = flashKLV.copyRecordsToOtherBankAndSwapBanks();
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    // check first sector of other bank erased after copy
    TEST_ASSERT_EQUAL(true, flashKLV.isSectorErasedOtherBank(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(7));

    // BANK_HEADER =  { 0xFF, 0xFE, 0x04, 0x00, 0x01, 0x02, 0x03, 0x05 };
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xF1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0xF2, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0xF3, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0xF4, flashKLV.flashPeek(7));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(13));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(15));

    FlashKLV flashKLV_B(&flashMemory2Banks[0], SECTOR_COUNT, FlashKLV::TWO_BANKS);
    TEST_ASSERT_EQUAL(flashKLV_B.getOtherBankMemoryPtr() + BANKB_OFFSET, flashKLV_B.getCurrentBankMemoryPtr());

    flashKLV.eraseOtherBank();
    err = flashKLV.copyRecordsToOtherBankAndSwapBanks();
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(true, flashKLV.isSectorErasedOtherBank(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xF1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0xF2, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0xF3, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0xF4, flashKLV.flashPeek(7));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flashPeek(13));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(15));
}

void test_copy_2()
{
    flashMemory2Banks.fill(0);
    FlashKLV flashKLV(&flashMemory2Banks[0], SECTOR_COUNT, FlashKLV::TWO_BANKS);
    flashKLV.eraseCurrentBank();

    int err {};
    record4A_t record4A {};
    record4B_t record4B {};

    record4A.value = 0x1A2B3C4D;
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(flashKLV.memorySize() - 6, flashKLV.bytesFree());
    TEST_ASSERT_EQUAL(false, flashKLV.isRecordEmpty(0));
    TEST_ASSERT_EQUAL(0x14, flashKLV.getRecordKey(0));
    TEST_ASSERT_EQUAL(4, flashKLV.getRecordLength(0));
    TEST_ASSERT_EQUAL(6, flashKLV.getRecordPositionIncrement(0));
    TEST_ASSERT_EQUAL_PTR(&flashMemory2Banks[2], flashKLV.getRecordValuePtr(0));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x4D, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x3C, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0x2B, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0x1A, flashKLV.flashPeek(5));

    record4A.value = 0x0ABBCCDD;
    err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(7));
    TEST_ASSERT_EQUAL(0xDD, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(0xCC, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(0xBB, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flashPeek(11));

    record4B.value = 0x11223344;
    err = flashKLV.write(record4B.key, record4B.length, &record4B.value);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_GREATER_OR_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(0x24 | TOP_BIT, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(13));
    TEST_ASSERT_EQUAL(0x44, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(0x33, flashKLV.flashPeek(15));
    TEST_ASSERT_EQUAL(0x22, flashKLV.flashPeek(16));
    TEST_ASSERT_EQUAL(0x11, flashKLV.flashPeek(17));

    TEST_ASSERT_EQUAL(false, flashKLV.isOtherBankErased());
    flashKLV.eraseOtherBank();
    TEST_ASSERT_EQUAL(true, flashKLV.isOtherBankErased());
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(7));

    TEST_ASSERT_EQUAL(512*2, BANKB_OFFSET);
    err = flashKLV.copyRecordsToOtherBankAndSwapBanks();
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);
    TEST_ASSERT_EQUAL(true, flashKLV.isSectorErasedOtherBank(0));
    // BANK_HEADER =  { 0xFF, 0xFE, 0x04, 0x00, 0xF1, 0xF2, 0xF3, 0xF4 };
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(0));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flashPeek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flashPeek(3));
    TEST_ASSERT_EQUAL(0xF1, flashKLV.flashPeek(4));
    TEST_ASSERT_EQUAL(0xF2, flashKLV.flashPeek(5));
    TEST_ASSERT_EQUAL(0xF3, flashKLV.flashPeek(6));
    TEST_ASSERT_EQUAL(0xF4, flashKLV.flashPeek(7));
    // check first sector of other bank erased after copy
    TEST_ASSERT_EQUAL(true, flashKLV.isSectorErasedOtherBank(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeekOther(7));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flashPeek(8));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(9));
    TEST_ASSERT_EQUAL(0xDD, flashKLV.flashPeek(10));
    TEST_ASSERT_EQUAL(0xCC, flashKLV.flashPeek(11));
    TEST_ASSERT_EQUAL(0xBB, flashKLV.flashPeek(12));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flashPeek(13));

    TEST_ASSERT_EQUAL(0x24 | TOP_BIT, flashKLV.flashPeek(14));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flashPeek(15));
    TEST_ASSERT_EQUAL(0x44, flashKLV.flashPeek(16));
    TEST_ASSERT_EQUAL(0x33, flashKLV.flashPeek(17));
    TEST_ASSERT_EQUAL(0x22, flashKLV.flashPeek(18));
    TEST_ASSERT_EQUAL(0x11, flashKLV.flashPeek(19));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(20));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flashPeek(21));
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-reinterpret-cast,hicpp-signed-bitwise,readability-identifier-length,readability-magic-numbers,altera-struct-pack-align)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_one_bank);
    RUN_TEST(test_two_banks);
    RUN_TEST(test_copy_1);
    RUN_TEST(test_copy_2);

    UNITY_END();
}
