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
enum { BANKB_OFFSET = FlashKlv::SECTOR_SIZE*SECTOR_COUNT };
static std::array<uint8_t, FlashKlv::SECTOR_SIZE*SECTOR_COUNT> flashMemory;
static std::array<uint8_t, FlashKlv::SECTOR_SIZE*SECTOR_COUNT*2> flashMemory2Banks;

// for FRAMEWORK_TEST, PAGE_SIZE = 8

enum : uint8_t { TOP_BIT = 0x80, WORD_BIT = 0x40, TOP_BITS = TOP_BIT|WORD_BIT };

struct record4A_t {
    uint16_t key = 0x14;
    uint16_t length = sizeof(int32_t);
    int32_t value = 0;
    std::span<const uint8_t> span{reinterpret_cast<const uint8_t*>(&value), sizeof(value)};
};

struct record4B_t {
    uint16_t key = 0x24;
    uint16_t length = sizeof(int32_t);
    int32_t value = 0;
    std::span<const uint8_t> span{reinterpret_cast<const uint8_t*>(&value), sizeof(value)};
};

void test_one_bank()
{
    flashMemory.fill(0);
    FlashKlv flashKLV(flashMemory, FlashKlv::ONE_BANK);
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

    TEST_ASSERT_EQUAL(flashKLV.memory_size(), flashKLV.bytes_free());
}

void test_two_banks()
{
    flashMemory2Banks.fill(0);
    FlashKlv flashKLV(flashMemory2Banks, FlashKlv::TWO_BANKS);
    TEST_ASSERT_EQUAL_PTR(flashKLV.get_current_bank_memory_slice().data() + BANKB_OFFSET, flashKLV.get_other_bank_memory_slice().data());
    TEST_ASSERT_EQUAL(false, flashKLV.is_current_bank_erased());
    TEST_ASSERT_EQUAL(false, flashKLV.is_other_bank_erased());

    flashKLV.erase_current_bank();
    TEST_ASSERT_EQUAL(true, flashKLV.is_current_bank_erased());
    TEST_ASSERT_EQUAL(flashKLV.memory_size(), flashKLV.bytes_free());

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(FlashKlv::SECTOR_SIZE*SECTOR_COUNT - 1));

    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(BANKB_OFFSET));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(BANKB_OFFSET + 8));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(2*FlashKlv::SECTOR_SIZE*SECTOR_COUNT - 1));
}

void test_copy_1()
{
    flashMemory2Banks.fill(0);
    FlashKlv flashKLV(flashMemory2Banks, FlashKlv::TWO_BANKS);
    TEST_ASSERT_EQUAL_PTR(flashKLV.get_current_bank_memory_slice().data() + BANKB_OFFSET, flashKLV.get_other_bank_memory_slice().data());
    TEST_ASSERT_EQUAL(false, flashKLV.is_current_bank_erased());
    TEST_ASSERT_EQUAL(false, flashKLV.is_other_bank_erased());
    flashKLV.erase_current_bank();
    TEST_ASSERT_EQUAL(true, flashKLV.is_current_bank_erased());
    TEST_ASSERT_EQUAL(false, flashKLV.is_other_bank_erased());

    int err {};
    record4A_t record4A {};

    record4A.value = 0x7B536AFE;
    //err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    err = flashKLV.write_key_value(record4A.key, record4A.span);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(flashKLV.memory_size() - 6, flashKLV.bytes_free());
    TEST_ASSERT_EQUAL(false, flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(0x14, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(4, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(6, flashKLV.get_record_position_increment(0));
    TEST_ASSERT_EQUAL(2, flashKLV.get_record_value_pos(0));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(5));

    TEST_ASSERT_EQUAL(false, flashKLV.is_other_bank_erased());
    flashKLV.erase_other_bank();
    TEST_ASSERT_EQUAL(true, flashKLV.is_other_bank_erased());
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(7));

    TEST_ASSERT_EQUAL(512*2, BANKB_OFFSET);
    err = flashKLV.copy_records_to_other_bank_and_swap_banks();
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    // check first sector of other bank erased after copy
    TEST_ASSERT_EQUAL(true, flashKLV.is_sector_erased_other_bank(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(7));

    // BANK_HEADER =  { 0xFF, 0xFE, 0x04, 0x00, 0x01, 0x02, 0x03, 0x05 };
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xF1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0xF2, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0xF3, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0xF4, flashKLV.flash_peek(7));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(13));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(15));

    FlashKlv flashKLV_B(flashMemory2Banks, FlashKlv::TWO_BANKS);
    TEST_ASSERT_EQUAL_PTR(flashKLV_B.get_other_bank_memory_slice().data() + BANKB_OFFSET, flashKLV_B.get_current_bank_memory_slice().data());

    flashKLV.erase_other_bank();
    err = flashKLV.copy_records_to_other_bank_and_swap_banks();
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(true, flashKLV.is_sector_erased_other_bank(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xF1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0xF2, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0xF3, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0xF4, flashKLV.flash_peek(7));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0x6A, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(0x53, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(0x7B, flashKLV.flash_peek(13));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(15));
}

void test_copy_2()
{
    flashMemory2Banks.fill(0);
    FlashKlv flashKLV(flashMemory2Banks, FlashKlv::TWO_BANKS);
    flashKLV.erase_current_bank();

    int err {};
    record4A_t record4A {};
    record4B_t record4B {};

    record4A.value = 0x1A2B3C4D;
    //err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    err = flashKLV.write_key_value(record4A.key, record4A.span);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(flashKLV.memory_size() - 6, flashKLV.bytes_free());
    TEST_ASSERT_EQUAL(false, flashKLV.is_record_empty(0));
    TEST_ASSERT_EQUAL(0x14, flashKLV.get_record_key(0));
    TEST_ASSERT_EQUAL(4, flashKLV.get_record_length(0));
    TEST_ASSERT_EQUAL(6, flashKLV.get_record_position_increment(0));
    TEST_ASSERT_EQUAL_PTR(2, flashKLV.get_record_value_pos(0));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x4D, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x3C, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0x2B, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0x1A, flashKLV.flash_peek(5));

    record4A.value = 0x0ABBCCDD;
    //err = flashKLV.write(record4A.key, record4A.length, &record4A.value);
    err = flashKLV.write_key_value(record4A.key, record4A.span);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(7));
    TEST_ASSERT_EQUAL(0xDD, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(0xCC, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(0xBB, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flash_peek(11));

    record4B.value = 0x11223344;
    //err = flashKLV.write(record4B.key, record4B.length, &record4B.value);
    err = flashKLV.write_key_value(record4B.key, record4B.span);
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_GREATER_OR_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(0x24 | TOP_BIT, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(13));
    TEST_ASSERT_EQUAL(0x44, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(0x33, flashKLV.flash_peek(15));
    TEST_ASSERT_EQUAL(0x22, flashKLV.flash_peek(16));
    TEST_ASSERT_EQUAL(0x11, flashKLV.flash_peek(17));

    TEST_ASSERT_EQUAL(false, flashKLV.is_other_bank_erased());
    flashKLV.erase_other_bank();
    TEST_ASSERT_EQUAL(true, flashKLV.is_other_bank_erased());
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(7));

    TEST_ASSERT_EQUAL(512*2, BANKB_OFFSET);
    err = flashKLV.copy_records_to_other_bank_and_swap_banks();
    TEST_ASSERT_EQUAL(FlashKlv::OK, err);
    TEST_ASSERT_EQUAL(true, flashKLV.is_sector_erased_other_bank(0));
    // BANK_HEADER =  { 0xFF, 0xFE, 0x04, 0x00, 0xF1, 0xF2, 0xF3, 0xF4 };
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(0));
    TEST_ASSERT_EQUAL(0xFE, flashKLV.flash_peek(1));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(2));
    TEST_ASSERT_EQUAL(0x00, flashKLV.flash_peek(3));
    TEST_ASSERT_EQUAL(0xF1, flashKLV.flash_peek(4));
    TEST_ASSERT_EQUAL(0xF2, flashKLV.flash_peek(5));
    TEST_ASSERT_EQUAL(0xF3, flashKLV.flash_peek(6));
    TEST_ASSERT_EQUAL(0xF4, flashKLV.flash_peek(7));
    // check first sector of other bank erased after copy
    TEST_ASSERT_EQUAL(true, flashKLV.is_sector_erased_other_bank(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(0));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(1));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(2));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(3));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(4));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(5));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(6));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek_other(7));

    TEST_ASSERT_EQUAL(0x14 | TOP_BIT, flashKLV.flash_peek(8));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(9));
    TEST_ASSERT_EQUAL(0xDD, flashKLV.flash_peek(10));
    TEST_ASSERT_EQUAL(0xCC, flashKLV.flash_peek(11));
    TEST_ASSERT_EQUAL(0xBB, flashKLV.flash_peek(12));
    TEST_ASSERT_EQUAL(0x0A, flashKLV.flash_peek(13));

    TEST_ASSERT_EQUAL(0x24 | TOP_BIT, flashKLV.flash_peek(14));
    TEST_ASSERT_EQUAL(0x04, flashKLV.flash_peek(15));
    TEST_ASSERT_EQUAL(0x44, flashKLV.flash_peek(16));
    TEST_ASSERT_EQUAL(0x33, flashKLV.flash_peek(17));
    TEST_ASSERT_EQUAL(0x22, flashKLV.flash_peek(18));
    TEST_ASSERT_EQUAL(0x11, flashKLV.flash_peek(19));

    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(20));
    TEST_ASSERT_EQUAL(0xFF, flashKLV.flash_peek(21));
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
