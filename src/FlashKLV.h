#pragma once

#include <array>
#include <cstddef>
#include <cstdint>


/*!
Flash Key Length Value (KLV) storage.

Currently only implemented on Raspberry Pi Pico platform.
*/
class FlashKlv {
public:
    enum { ONE_BANK = 1, TWO_BANKS = 2 };
    enum { OVERWRITE_RECORDS = 0x01, DELETE_RECORDS = 0x02, USE_CRC = 0x04 };
    static constexpr uint32_t OK = 0;
    static constexpr uint32_t OK_NO_NEED_TO_WRITE = 1;
    static constexpr uint32_t OK_OVERWRITTEN = 2;
    static constexpr uint32_t OK_NOTHING_TO_COPY = 3;
    static constexpr uint32_t OK_SECTOR_ALREADY_ERASED = 4;
    static constexpr uint32_t OK_NO_RECORDS_COPIED = 5;

    enum {
        ERROR_FLASH_FULL = -1, ERROR_NOT_FOUND = -2, ERROR_INVALID_KEY = -3,
        ERROR_RECORD_TOO_LARGE = -4, ERROR_RECORD_TOO_SMALL = -5,
        ERROR_OTHER_BANK_NOT_INITIALIZED = -6, ERROR_OTHER_BANK_NOT_ERASED = -7,
        ERROR_INVALID_FLASH_BANK_PTR = -8
    };
    static constexpr uint32_t ERROR_NO_FREE_FLASH = UINT32_MAX;
    static constexpr uint16_t NOT_FOUND = 0;
    static constexpr uint16_t RECORD_KEY_EMPTY = 0xFFFF; 
    static constexpr uint16_t RECORD_KEY_BANK_HEADER = 0x3FFE;
    static constexpr uint16_t RECORD_KEY_DELETED = 0;
    static constexpr uint16_t RECORD_EMPTY = 0xFF;
    static constexpr uint8_t  KEY8_MIN = 0x01;
    static constexpr uint8_t  KEY8_MAX = 0x3F;
    static constexpr uint16_t KEY16_MIN = 0x0100;
    static constexpr uint16_t KEY16_MAX = 0x3FFD;
#if defined(FRAMEWORK_TEST)
    static constexpr size_t SECTOR_SIZE = 512;  //!< Minimum erasable amount, bytes.
    static constexpr size_t PAGE_SIZE = 8; //!< Minimum writable amount, bytes.
#else
    static constexpr size_t SECTOR_SIZE = 4096;  //!< Minimum erasable amount, bytes.
    static constexpr size_t PAGE_SIZE = 256; //!< Minimum writable amount, bytes.
#endif
protected:
    enum : uint8_t { DELETED_MASK = 0x7F, UNDELETED_BIT = 0x80, KL16_BIT = 0x40 };
    enum { NO_DELETE = SIZE_MAX };
public:
    struct klv_t {
        uint16_t key;
        uint16_t length;
        const uint8_t* value_ptr;
    };
    struct kl8_t { // key and length for 8-bit KLV record
        uint8_t key;
        uint8_t length;
    };
    struct kl16_t { // key and length for 16-bit KLV record
        uint16_t key;
        uint16_t length;
    };
    struct record_count_t {
        size_t record_count;
        size_t deleted_record_count;
    };
protected:
    struct erase_params_t {
        uint8_t* address;
        size_t count;
    };
    struct program_params_t {
        uint8_t* address;
        uint8_t* data;
    };
public:
    FlashKlv(uint8_t* flash_memory_ptr, size_t sectors_per_bank, size_t bankCount);
    FlashKlv(uint8_t* flash_memory_ptr, size_t sectors_per_bank);
    FlashKlv(size_t sectors_per_bank, size_t bankCount);
    explicit FlashKlv(size_t sectors_per_bank);
    ~FlashKlv() = default;
public:
    static uint8_t calculate_crc(uint8_t crc, uint8_t value);
    static uint8_t calculate_crc(uint8_t crc, const uint8_t *data, size_t length);

    static bool key_ok(uint16_t key) { return ((key >= KEY8_MIN && key <= KEY8_MAX) || (key >= KEY16_MIN && key <= KEY16_MAX)); }

    size_t bytes_free() const;
    size_t memory_size() const { return _bank_memory_size; }

    klv_t find(uint16_t key) const;
    klv_t find_next(size_t pos) const;
    size_t find_first_free_pos() const;
    record_count_t count_records() const;
    int32_t copy_records_to_other_bank_and_swap_banks();
    int32_t read(void* value, size_t size,  uint16_t key) const;

    int32_t remove(uint16_t key) { return remove(key, _current_bank_memory_ptr); }
    int32_t write(uint16_t key, uint16_t length, const uint8_t* value_ptr) { return write(key, length, value_ptr, _current_bank_memory_ptr); }
    int32_t write(uint16_t key, uint16_t length, const void* value_ptr) { return write(key, length, static_cast<const uint8_t*>(value_ptr)); }
    int32_t write(const klv_t& klv) { return write(klv.key, klv.length, klv.value_ptr); }

    bool is_current_bankErased() const  { return is_bank_erased(_current_bank_memory_ptr); }
    bool is_other_bankErased() const  { return is_bank_erased(_other_bank_memory_ptr); }
    int32_t erase_current_bank() { return erase_bank(_current_bank_memory_ptr); }
    int32_t erase_other_bank() { return erase_bank(_other_bank_memory_ptr); }
    size_t get_bank_sector_count() const { return _bank_sector_count; }
    bool is_sector_erased_current_bank(size_t sector) const  { return is_sector_erased(sector, _current_bank_memory_ptr); }
    bool is_sector_erased_other_bank(size_t sector) const  { return is_sector_erased(sector, _other_bank_memory_ptr); }

    bool overwrite_records() const { return _mode & OVERWRITE_RECORDS; }
    bool delete_records() const { return _mode & DELETE_RECORDS; }
    bool use_crc() const { return _mode & USE_CRC; }
protected:
    static bool is_sector_erased(size_t sector, const uint8_t* flash_bank_memory_ptr);
    bool is_bank_erased(const uint8_t* flash_bank_memory_ptr) const;
    int32_t erase_bank(uint8_t* flash_bank_memory_ptr);
    static int32_t erase_sector(size_t sector, uint8_t* flash_bank_memory_ptr);

    int32_t remove(uint16_t key, uint8_t* flash_memory_ptr);
    int32_t write(uint16_t key, uint16_t length, const uint8_t* value_ptr, uint8_t* flash_memory_ptr);
    void flash_mark_record_as_deleted(size_t pos, uint8_t* flash_memory_ptr);
    void flash_write(size_t pos, uint16_t length, const uint8_t* value_ptr, uint8_t* flash_memory_ptr);
    void flash_delete_and_write(size_t deletePos, size_t pos, uint16_t key, uint16_t length, const uint8_t* value_ptr, uint8_t* flash_memory_ptr);
    void flash_read_page(size_t pageIndex, const uint8_t* flash_memory_ptr);
    void flash_write_page(size_t pageIndex, uint8_t* flash_memory_ptr);

    static bool is_record_empty(size_t pos, const uint8_t* flash_memory_ptr);
    static uint16_t get_record_key(size_t pos, const uint8_t* flash_memory_ptr);
    static uint16_t get_record_length(size_t pos, const uint8_t* flash_memory_ptr);
    static uint16_t get_record_position_increment(size_t pos, const uint8_t* flash_memory_ptr);
    static const uint8_t* get_record_value_ptr(size_t pos, const uint8_t* flash_memory_ptr);

    static void call_flash_range_erase(void* param);
    static void call_flash_range_program(void* param);
public:
// internal, exposed for testing
    static bool overwriteable(uint8_t flash, uint8_t value);
    static bool overwriteable(const uint8_t* flashPtr, const uint8_t* value_ptr, uint16_t length);
    bool is_record_empty(size_t pos) const { return is_record_empty(pos, _current_bank_memory_ptr); }
    static bool is_empty(uint16_t flash_record_key);
    uint16_t get_record_key(size_t pos) const { return get_record_key(pos, _current_bank_memory_ptr); }
    uint16_t get_record_length(size_t pos) const { return get_record_length(pos, _current_bank_memory_ptr); }
    uint16_t get_record_position_increment(size_t pos) const { return get_record_position_increment(pos, _current_bank_memory_ptr); }
    const uint8_t* get_record_value_ptr(size_t pos) const { return get_record_value_ptr(pos, _current_bank_memory_ptr); }
    int32_t copy_records_to_other_bank();
    void swap_banks() { uint8_t* other_bank_memory_ptr = _other_bank_memory_ptr; _other_bank_memory_ptr = _current_bank_memory_ptr; _current_bank_memory_ptr = other_bank_memory_ptr; }
// for testing
    const uint8_t* flash_pos(size_t pos) { return _current_bank_memory_ptr + pos; } //!< for testing
    uint8_t flash_peek(size_t pos) { return _current_bank_memory_ptr[pos]; } //!< for testing
    uint8_t flash_peek_other(size_t pos) { return _other_bank_memory_ptr[pos]; } //!< for testing
    const uint8_t* get_current_bankMemory_ptr() const { return _current_bank_memory_ptr; }
    const uint8_t* get_other_bank_memory_ptr() const { return _other_bank_memory_ptr; }
    int32_t erase_sector(size_t sector) { return erase_sector(sector, _current_bank_memory_ptr); }
protected:
    uint8_t* _flash_base_memory_ptr;
    uint8_t* _current_bank_memory_ptr;
    uint8_t* _other_bank_memory_ptr {};
    size_t _bank_memory_size; //!< the size of each memory bank
    size_t _bank_sector_count; //!< the number of sectors in each memory bank
    uint32_t _mode {OVERWRITE_RECORDS | DELETE_RECORDS};
    std::array<uint8_t, PAGE_SIZE> _page_cache {};
    static constexpr std::array<uint8_t, 8> BANK_HEADER =  { 0xFF, 0xFE, 0x04, 0x00, 0xF1, 0xF2, 0xF3, 0xF4 };
};
