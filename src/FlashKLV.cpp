#include "FlashKLV.h"
#include <cassert>
#include <cstring>


#if defined(FRAMEWORK_USE_FREERTOS) || defined(PIO_FRAMEWORK_ARDUINO_ENABLE_FREERTOS)
#if defined(FRAMEWORK_ESPIDF) || defined(FRAMEWORK_ARDUINO_ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#else
#if defined(FRAMEWORK_ARDUINO_STM32)
#include <STM32FreeRTOS.h>
#endif
#include <FreeRTOS.h>
#include <task.h>
#endif
#endif

#if defined(FRAMEWORK_RPI_PICO)

#elif defined(FRAMEWORK_ESPIDF)

#elif defined(FRAMEWORK_STM32_CUBE)

#elif defined(FRAMEWORK_TEST)

int flash_safe_execute(void (*fn)(void*), void* param, uint32_t timeout_ms) { (void)timeout_ms; fn(param); return 0; }

#else // defaults to FRAMEWORK_ARDUINO

#if defined(FRAMEWORK_ARDUINO_RPI_PICO)
#elif defined(FRAMEWORK_ARDUINO_ESP32)
#elif defined(FRAMEWORK_ARDUINO_STM32)
#else
// No FRAMEWORK defined, so default to FRAMEWORK_ARDUINO_RPI_PICO so this code can be used within Arduino IDE
//#define FRAMEWORK_ARDUINO_RPI_PICO
#endif // FRAMEWORK_ARDUINO

#endif // FRAMEWORK

#if defined(FRAMEWORK_RPI_PICO) || defined(FRAMEWORK_ARDUINO_RPI_PICO)
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <pico/flash.h>
#endif

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays,modernize-avoid-c-arrays)

/*!
If bankCount == 1 then we have one bank of flash at
address: flash_memory_slice, size: SECTOR_SIZE*sectors_per_bank

If bankCount == 2 then we have
BANK_A address: flash_memory_slice, size: SECTOR_SIZE*sectors_per_bank
BANK_B address: flash_memory_slice + SECTOR_SIZE*sectors_per_bank, size: SECTOR_SIZE*sectors_per_bank
*/

FlashKlv::FlashKlv(uint8_t* flash_memory_slice, size_t sectors_per_bank, size_t bankCount) :
    _flash_base_memory_slice(flash_memory_slice, sectors_per_bank * SECTOR_SIZE * bankCount),
    _current_bank_memory_slice(flash_memory_slice, sectors_per_bank * SECTOR_SIZE),
    _other_bank_memory_slice(flash_memory_slice + sectors_per_bank * SECTOR_SIZE, sectors_per_bank * SECTOR_SIZE),
    _bank_memory_size(sectors_per_bank * SECTOR_SIZE),
    _bank_sector_count(sectors_per_bank)
{
    assert(bankCount == 1 || bankCount == 2);

    /*if (bankCount == 2) {
        uint8_t* BANK_A_PTR = flash_memory_slice;
        uint8_t* BANK_B_PTR = flash_memory_slice + _bank_memory_size; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        // if the BANK_B has data in it, then that should be the current bank
        if (memcmp(BANK_B_PTR, &BANK_HEADER[0], sizeof(BANK_HEADER)) == 0) {
            _current_bank_memory_slice = BANK_B_PTR;
            _other_bank_memory_slice = BANK_A_PTR;
        } else {
            _current_bank_memory_slice = BANK_A_PTR;
            _other_bank_memory_slice = BANK_B_PTR;
        }
    }*/
#if defined(FRAMEWORK_RPI_PICO) || defined(FRAMEWORK_ARDUINO_RPI_PICO)
    static_assert(FLASH_SECTOR_SIZE == SECTOR_SIZE);
    static_assert(FLASH_PAGE_SIZE == PAGE_SIZE);
#endif
}

/*!
For RP2040 PICO_FLASH_SIZE_BYTES is typically 2MB (2097152 bytes).
For RP2350 it is typically 4MB
*/
FlashKlv::FlashKlv(size_t sectors_per_bank, size_t bankCount) : // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
#if defined(FRAMEWORK_RPI_PICO) || defined(FRAMEWORK_ARDUINO_RPI_PICO)
    FlashKlv(reinterpret_cast<uint8_t*>(XIP_BASE + PICO_FLASH_SIZE_BYTES - SECTOR_SIZE*sectors_per_bank*bankCount), sectors_per_bank, bankCount)
#else
    FlashKlv(nullptr, sectors_per_bank, bankCount)
#endif
{
}

FlashKlv::FlashKlv(uint8_t flash_memory_slice[], size_t sectors_per_bank) :
    FlashKlv(flash_memory_slice, sectors_per_bank, ONE_BANK) {}

FlashKlv::FlashKlv(size_t sectors_per_bank) :
    FlashKlv(sectors_per_bank, ONE_BANK) {}


uint8_t FlashKlv::calculate_crc(uint8_t crc, uint8_t value)
{
    static constexpr uint8_t POLYNOMIAL = 0xD5;

    crc ^= value;
    for (int ii = 0; ii < 8; ++ii) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        const uint8_t top_bit = crc & 0x80U; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        crc <<= 1U;
        if (top_bit != 0) {
            crc ^= POLYNOMIAL;
        }
    }
    return crc;
}

uint8_t FlashKlv::calculate_crc(uint8_t crc, const uint8_t *data, size_t length)
{
    for (const uint8_t* end = data + length; data != end; ++data) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        crc = calculate_crc(crc, *data);
    }
    return crc;
}



/*!
Flash can be overwritten if bits are only flipped from 1 to 0, never from 0 to 1
*/
bool FlashKlv::is_byte_overwriteable(uint8_t flash, uint8_t value)
{
    for (uint8_t bit = 0x80; bit !=0; bit >>= 1U) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        if (((flash & bit) == 0) && ((value & bit) == bit)) {
            return false;
        }
    }
    return true;
}

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
bool FlashKlv::is_slice_overwriteable(const uint8_t* flash_ptr, const uint8_t* value_ptr, uint16_t length)
{
    for (size_t ii = 0; ii < length; ++ ii) {
        const uint8_t flash = *flash_ptr++;
        const uint8_t value = *value_ptr++;

        for (uint8_t bit = 0x80; bit !=0; bit >>= 1U) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            if (((flash & bit) == 0) && ((value & bit) == bit)) {
                return false;
            }
        }
    }
    return true;
}

uint16_t FlashKlv::get_record_key_slice(size_t pos, const std::span<const uint8_t>& flash_memory_slice)
//get_record_key_slice(size_t pos, const uint8_t flash_memory_slice[])
{
    const uint8_t key8 = flash_memory_slice[pos];
    if ((key8 & UNDELETED_BIT) == 0) {
        return RECORD_KEY_DELETED;
    }
    if ((key8 & KL16_BIT) == 0) {
        enum : uint8_t { KEY8_MASK = 0x3F };
        return key8 & KEY8_MASK;
    }
    // note byte order is reversed, this is done because and empty record is denoted by setting the high bit of the high byte to zero.
    uint16_t key16 = key8;
    key16 = (key16 << 8U) | flash_memory_slice[pos + 1]; // NOLINT(cppcoreguidelines-avoid-magic-numbers,hicpp-signed-bitwise,readability-magic-numbers)
    if (key16 == RECORD_KEY_EMPTY) {
        return RECORD_KEY_EMPTY;
    }
    enum : uint16_t { KEY16_MASK = 0x3FFF };
    return key16 & KEY16_MASK;
}

bool FlashKlv::is_record_empty_slice(size_t pos, const std::span<const uint8_t>& flash_memory_slice)
{
    const uint8_t octet = flash_memory_slice[pos];
    return (octet == RECORD_EMPTY) ? true : false; // NOLINT(readability-simplify-boolean-expr)
}

uint16_t FlashKlv::get_record_length_slice(size_t pos, const std::span<const uint8_t>& flash_memory_slice)
{
    if ((flash_memory_slice[pos] & KL16_BIT) == 0) {
        return flash_memory_slice[pos + sizeof(kl8_t::key)];
    }
    const uint16_t lengthL = flash_memory_slice[pos + sizeof(kl16_t::key)];
    const uint16_t lengthH = flash_memory_slice[pos + sizeof(kl16_t::key) + 1];
    return lengthL | (lengthH << 8U); // NOLINT(cppcoreguidelines-avoid-magic-numbers,hicpp-signed-bitwise,readability-magic-numbers)
}

uint16_t FlashKlv::get_record_position_increment_slice(size_t pos, const std::span<const uint8_t>& flash_memory_slice)
{
    const size_t offset = ((flash_memory_slice[pos] & KL16_BIT) == 0) ? sizeof(kl8_t) : sizeof(kl16_t);
    const uint16_t ret = get_record_length_slice(pos, flash_memory_slice) + static_cast<uint16_t>(offset);
    return ret;
}

const uint8_t* FlashKlv::get_record_value_ptr_slice(size_t pos, const std::span<const uint8_t>& flash_memory_slice)
{
    if ((flash_memory_slice[pos] & KL16_BIT) == 0) {
        return &flash_memory_slice[pos + sizeof(kl8_t)];
    }
    return &flash_memory_slice[pos + sizeof(kl16_t)];
}

// function for transitioning to using value_pos rather than value_ptr
size_t FlashKlv::get_record_value_pos_slice(size_t pos, const std::span<const uint8_t>& flash_memory_slice)
{
    if ((flash_memory_slice[pos] & KL16_BIT) == 0) {
        return pos + sizeof(kl8_t);
    }
    return pos + sizeof(kl16_t);
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

bool FlashKlv::is_empty(uint16_t flash_record_key)
{
    return flash_record_key == RECORD_KEY_EMPTY;
}

size_t FlashKlv::find_first_free_pos() const
{
    size_t pos = 0;
    uint16_t flash_record_key = get_record_key(pos);
    while (!is_empty(flash_record_key)) {
        pos += get_record_position_increment(pos);
        if (pos >= _bank_memory_size) {
            return ERROR_NO_FREE_FLASH;
        }
        flash_record_key = get_record_key(pos);
    }

    return pos;
}

size_t FlashKlv::bytes_free() const
{
    const size_t firstFreePos = find_first_free_pos();
    return (firstFreePos == ERROR_NO_FREE_FLASH) ?  0 : _bank_memory_size - firstFreePos;
}

/*!
Remove the record with the specified key by marking it as deleted.
*/
int32_t FlashKlv::remove(uint16_t key, std::span<uint8_t>& flash_memory_slice)
{
    if (!key_ok(key)) {
        return ERROR_INVALID_KEY;
    }

    size_t pos = 0;
    uint16_t flash_record_key = get_record_key_slice(pos, flash_memory_slice);
    while (!is_empty(flash_record_key)) {
        if (flash_record_key == key) {
            flash_mark_record_as_deleted(pos, flash_memory_slice);
            return OK;
        }
        pos += get_record_position_increment(pos);
        if (pos >= _bank_memory_size) {
            return ERROR_NOT_FOUND;
        }
        flash_record_key = get_record_key(pos);
    }
    return ERROR_NOT_FOUND;
}

/*!
Find the record with the specified key.

Find always searches the current bank.
*/
FlashKlv::klv_t FlashKlv::find(uint16_t key) const
{
    klv_t klv = {.key = NOT_FOUND, .length = 0, .value_ptr = nullptr};
    if (!key_ok(key)) {
        return klv;
    }

    // Walk the flash until skipping over deleted records and records with a different key.
    size_t pos = 0;
    uint16_t flash_record_key = get_record_key(pos);
    while (!is_empty(flash_record_key)) {
        if (flash_record_key == key) {
            klv = {.key = key, .length = get_record_length(pos), .value_ptr = get_record_value_ptr(pos)};
            if (delete_records()) {
                // if records are deleted, then there will be only one record with the given key
                return klv;
            }
            // records are not deleted, so continue searching to see if there is another record with this key
        }
        pos += get_record_position_increment(pos);
        if (pos >= _bank_memory_size) {
            return klv;
        }
        flash_record_key = get_record_key(pos);
    }
    return klv;
}

/*!
Find the next undeleted record with any key, starting at pos.

Find always searches the current bank.
*/
FlashKlv::klv_t FlashKlv::find_next(size_t pos) const
{
    // skip over the record at pos
    uint16_t flash_record_key = get_record_key(pos);
    if (is_empty(flash_record_key)) {
        return klv_t {.key = NOT_FOUND, .length = 0, .value_ptr = nullptr};
    }
    pos += get_record_position_increment(pos);

    // now walk the flash to find the next undeleted record
    flash_record_key = get_record_key(pos);
    while (!is_empty(flash_record_key)) {
        if (flash_record_key != RECORD_KEY_DELETED) {
            return klv_t {.key = flash_record_key, .length = get_record_length(pos), .value_ptr = get_record_value_ptr(pos)};
        }
        pos += get_record_position_increment(pos);
        if (pos >= _bank_memory_size) {
            return klv_t {.key = NOT_FOUND, .length = 0, .value_ptr = nullptr};
        }
        flash_record_key = get_record_key(pos);
    }
    return klv_t {.key = NOT_FOUND, .length = 0, .value_ptr = nullptr};
}

FlashKlv::record_count_t FlashKlv::count_records() const
{
    record_count_t count{ .record_count = 0, .deleted_record_count = 0 };

    // walk the flash to count the records
    size_t pos = 0;
    uint16_t flash_record_key = get_record_key(pos);
    while (!is_empty(flash_record_key)) {
        if (flash_record_key == RECORD_KEY_DELETED) {
            ++count.deleted_record_count;
        } else {
            ++count.record_count;
        }
        pos += get_record_position_increment(pos);
        if (pos >= _bank_memory_size) {
            return count;
        }
        flash_record_key = get_record_key(pos);
    }
    return count;
}

/*!
Copy all undeleted records to the other bank.
*/
int32_t FlashKlv::copy_records_to_other_bank()
{
    //!!if (_other_bank_memory_slice == nullptr) {
    //    return ERROR_OTHER_BANK_NOT_INITIALIZED;
    //}

    if (!is_other_bank_erased()) {
        return ERROR_OTHER_BANK_NOT_ERASED;
    }

    size_t write_pos = sizeof(BANK_HEADER);
    flash_read_page(0, _other_bank_memory_slice);
    memcpy(&_page_cache[0], &BANK_HEADER[0], sizeof(BANK_HEADER));
    flash_write_page(0, _other_bank_memory_slice);

    size_t pos = 0;
    uint16_t flash_record_key = get_record_key(pos);
    if (is_empty(flash_record_key)) {
        return OK_NOTHING_TO_COPY;
    }

    while (!is_empty(flash_record_key)) {
        if (flash_record_key != RECORD_KEY_DELETED && flash_record_key != RECORD_KEY_BANK_HEADER) {
            // not a deleted record, so write it to the other bank
            flash_delete_and_write(NO_DELETE, write_pos, flash_record_key, get_record_length(pos), get_record_value_ptr(pos), _other_bank_memory_slice);
            write_pos += get_record_position_increment(pos);
        }
        pos += get_record_position_increment(pos);
        if (pos >= _bank_memory_size) {
            return ERROR_FLASH_FULL;
        }
        flash_record_key = get_record_key(pos);
    }
    return write_pos > sizeof(BANK_HEADER) ? OK : OK_NO_RECORDS_COPIED;
}

/*!
Copy all undeleted records to the other bank, and switch to using the other bank.
*/
int32_t FlashKlv::copy_records_to_other_bank_and_swap_banks()
{
    const int32_t err = copy_records_to_other_bank();
    if (err < 0) {
        return err;
    }
    swap_banks();
    // erase the first sector of the other bank, so that it will not be chosen as the current bank on the next FlashKlv construction
    erase_sector(0, _other_bank_memory_slice);

    return err;
}

/*!
Read the record with the given key.

Read always reads the current bank.
*/
int32_t FlashKlv::read(void* value, size_t size, uint16_t key) const
{
    if (!key_ok(key)) {
        return ERROR_INVALID_KEY;
    }

    const FlashKlv::klv_t klv = find(key);
    if (klv.key == NOT_FOUND) {
        return ERROR_NOT_FOUND;
    }
    if (klv.length > size) {
        memcpy(value, klv.value_ptr, size);
        return ERROR_RECORD_TOO_LARGE;
    }
    memcpy(value, klv.value_ptr, klv.length);
    if (klv.length < size) {
        return ERROR_RECORD_TOO_SMALL;
    }
    return OK;
}

/*
Write a record

Write to either the current bank or the other bank, as defined by flash_memory_slice.
*/
int32_t FlashKlv::write_klv_slice(uint16_t key, uint16_t length, const uint8_t* value_ptr, std::span<uint8_t>& flash_memory_slice)
{
    if (!key_ok(key)) {
        return ERROR_INVALID_KEY;
    }

    size_t pos = 0;
    size_t delete_pos = NO_DELETE;

    // look for an empty position to write the new record
    uint16_t flash_record_key = get_record_key(pos);
    while (!is_empty(flash_record_key)) {
        if (flash_record_key == key) {
            // there is already a record of this key, so first check if can be reused
            if (get_record_length(pos) == length) {
                // new record is same length as old, so check if it has changed
                if (!memcmp(get_record_value_ptr(pos), value_ptr, length)) {
                    // record has not changed, so no need to write it
                    return OK_NO_NEED_TO_WRITE;
                }
                // record has changed, so check if it can the be overwritten, that is bits are only flipped from 1 to 0, never from 0 to 1
                if (overwrite_records() && is_slice_overwriteable(get_record_value_ptr(pos), value_ptr, length)) {
                    // just overwrite the value: key and length are unchanged
                    flash_write(pos + (((flash_memory_slice[pos] & KL16_BIT) == 0) ? sizeof(kl8_t) : sizeof(kl16_t)), length, value_ptr, flash_memory_slice); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    return OK_OVERWRITTEN;
                }
            }
            // record has changed, so save the old one's position for later deletion and continue looking for empty position
            delete_pos = pos;
        }

        pos += get_record_position_increment(pos);
        if (pos + length > _bank_memory_size) {
            // not enough space for the new record
            return ERROR_FLASH_FULL;
        }
        flash_record_key = get_record_key(pos);
    }

    // we've found an empty position, so write the record there
    flash_delete_and_write(delete_pos, pos, key, length, value_ptr, flash_memory_slice);
    return OK;
}

/*!
Check if a sector of flash is erased (ie every byte has the value `0xFF`)
*/
bool FlashKlv::is_sector_erased(size_t sector, const std::span<const uint8_t>& flash_bank_memory_slice)
{
    enum { BYTE_ERASED = 0xFF };

    for (size_t ii = 0; ii < SECTOR_SIZE; ++ii) {
        if (flash_bank_memory_slice[ii + sector*SECTOR_SIZE] != BYTE_ERASED) {
            return false;
        }
    }
    return true;
}

/*!
Check if a bank of flash is erased (ie every byte has the value `0xFF`)
*/
bool FlashKlv::is_bank_erased(const std::span<const uint8_t>& flash_bank_memory_slice) const
{
    enum { BYTE_ERASED = 0xFF };
    for (size_t ii = 0; ii < _bank_memory_size; ++ii) {
        if (flash_bank_memory_slice[ii] != BYTE_ERASED) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return false;
        }
    }
    return true;
}

/*!
Erase an entire bank of flash, erasing in sector sized chunks.
*/
int32_t FlashKlv::erase_bank(std::span<uint8_t>& flash_bank_memory_slice) // NOLINT(readability-make-member-function-const)
{
    for (uint32_t ii = 0; ii < _bank_sector_count; ++ii) {
        erase_sector(ii, flash_bank_memory_slice);
    }
    return OK;
}

/*!
Mark the record at `pos` as deleted by setting bit 7 to zero.
*/
void FlashKlv::flash_mark_record_as_deleted(size_t pos, std::span<uint8_t>& flash_memory_slice)
{
    const size_t page_index = pos / PAGE_SIZE;
    flash_read_page(page_index, flash_memory_slice);

    const size_t page_offset = pos - page_index*PAGE_SIZE;
    _page_cache[page_offset] &= DELETED_MASK;

    flash_write_page(page_index, flash_memory_slice);
}

/*!
Read flash memory into the page cache.
*/
void FlashKlv::flash_read_page(size_t page_index, const std::span<const uint8_t>& flash_bank_memory_slice)
{
    //!!memcpy(&_page_cache[0], &flash_memory_slice[page_index*PAGE_SIZE], PAGE_SIZE);
    std::copy(flash_bank_memory_slice.begin() + page_index*PAGE_SIZE,
        flash_bank_memory_slice.begin() + page_index*PAGE_SIZE + PAGE_SIZE,
        &_page_cache[0]); 
}

/*!
write the data at `value_ptr` to flash in page-sized chunks using `flash_write_page`
*/
void FlashKlv::flash_write(size_t pos, uint16_t length, const uint8_t* value_ptr, std::span<uint8_t>& flash_memory_slice)
{
    while (length != 0) {
        const size_t page_index = pos / PAGE_SIZE;
        flash_read_page(page_index, flash_memory_slice);

        const size_t page_offset = pos - page_index*PAGE_SIZE;
        const size_t bytes_to_end_of_page = PAGE_SIZE - page_offset;

        if (length <= bytes_to_end_of_page) {
            // everything can fit on the current page, so copy it and return
            memcpy(&_page_cache[page_offset], value_ptr, length);
            flash_write_page(page_index, flash_memory_slice);
            return;
        }
        // copy what we have room for and then loop round for the next chunk
        memcpy(&_page_cache[page_offset], value_ptr, bytes_to_end_of_page);
        flash_write_page(page_index, flash_memory_slice);

        pos += bytes_to_end_of_page;
        value_ptr += bytes_to_end_of_page; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        length -= static_cast<uint16_t>(bytes_to_end_of_page);
    }
}

/*!
Mark the current instance of a record as deleted if `delete_pos != NO_DELETE` and then
write the new record in page-sized chunks using `flash_write_page`
*/
void FlashKlv::flash_delete_and_write(size_t delete_pos, size_t pos, uint16_t key, uint16_t length, const uint8_t* value_ptr, std::span<uint8_t>& flash_memory_slice)
{
    size_t page_index = pos / PAGE_SIZE;

    const size_t deletepage_index = ((delete_pos == NO_DELETE) || !delete_records()) ? NO_DELETE : delete_pos / PAGE_SIZE;
    if (delete_pos != NO_DELETE && deletepage_index != page_index) {
        // there is a record to mark as deleted, and it is in a different page than the record being written
        flash_mark_record_as_deleted(delete_pos, flash_memory_slice);
    }

    flash_read_page(page_index, flash_memory_slice);
    if (deletepage_index == page_index) {
        // if there is a deleted record, then mark it as deleted
        // note that delete_pos < pos, so there is no danger of this being overwritten
        _page_cache[delete_pos - page_index*PAGE_SIZE] &= DELETED_MASK;
    }

    size_t page_offset = pos - page_index*PAGE_SIZE;
    size_t bytes_to_end_of_page = PAGE_SIZE - page_offset;

    if (key <= KEY8_MAX) {
        key |= UNDELETED_BIT;
    } else {
        // reverse key byte order, this is done because and empty record is denoted by setting the high byte to zero.
        key = (key << 8U) | (key >> 8U) | UNDELETED_BIT | KL16_BIT; // NOLINT(cppcoreguidelines-avoid-magic-numbers,hicpp-signed-bitwise,readability-magic-numbers)
    }
    const kl8_t key_length8 = { .key = static_cast<uint8_t>(key), .length = static_cast<uint8_t>(length) };
    const kl16_t key_length16 = { .key = key, .length = length };
    const auto* key_length_ptr = ((key&KL16_BIT) == 0) ? reinterpret_cast<const uint8_t*>(&key_length8) : reinterpret_cast<const uint8_t*>(&key_length16); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    const size_t sizeof_key_length = ((key&KL16_BIT) == 0) ? sizeof(kl8_t) : sizeof(kl16_t);

    // update the pageCache with key and length, dealing with the case where key and length span two pages
    if (sizeof_key_length >= bytes_to_end_of_page) {
        // key and length fill this page
        // so copy up to the end of this page
        memcpy(&_page_cache[page_offset], key_length_ptr, bytes_to_end_of_page);
        flash_write_page(page_index++, flash_memory_slice);
        flash_read_page(page_index, flash_memory_slice);
        if (sizeof_key_length > bytes_to_end_of_page) {
            // key and length spill onto next page
            // so copy the rest onto the next page
            memcpy(&_page_cache[0], key_length_ptr + bytes_to_end_of_page, sizeof_key_length - bytes_to_end_of_page);
        }
    } else {
        // key and length are contained within this page
        memcpy(&_page_cache[page_offset], key_length_ptr, sizeof_key_length);
    }

    // now deal with the value
    pos += sizeof_key_length;
    page_offset = pos - page_index*PAGE_SIZE;
    bytes_to_end_of_page = PAGE_SIZE - page_offset;
    if (length <= bytes_to_end_of_page) {
        // the whole of the rest of the record fits in this page
        memcpy(&_page_cache[page_offset], value_ptr, length);
        flash_write_page(page_index, flash_memory_slice);
        return;
    }

    // copy the part of the record that fits on this page and write it
    memcpy(&_page_cache[page_offset], value_ptr, bytes_to_end_of_page);
    flash_write_page(page_index, flash_memory_slice);

    // deal with the rest of the record
    pos += bytes_to_end_of_page;
    value_ptr += bytes_to_end_of_page; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    length -= static_cast<uint16_t>(bytes_to_end_of_page);
    flash_write(pos, length, value_ptr, flash_memory_slice);
}


/*
PLATFORM SPECIFIC CODE

On Raspberry Pi Pico Flash is "execute in place" and so will be in use when any code that is stored in flash runs,
e.g. an interrupt handler or code running on a different core.

The code below ensures this will not happen, either by disabling interrupts or using `flash_safe_execute`, depending on the platform.

Calling `flash_range_erase` or `flash_range_program` at the same time as flash is running code would cause a crash.

`flash_safe_execute` disables interrupts and tries to cooperate with the other core to ensure flash is not in use
See the documentation for `flash_safe_execute` and its assumptions and limitations
*/

/*!
This function is called via the `flash_safe_execute()` wrapper function
*/
void FlashKlv::call_flash_range_erase(void* param)
{
    const erase_params_t* params = reinterpret_cast<erase_params_t*>(param); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
#if defined(FRAMEWORK_RPI_PICO)
    flash_range_erase(reinterpret_cast<uint32_t>(params->address) - XIP_BASE, params->count);
#elif defined(FRAMEWORK_TEST)
    enum { BYTE_ERASED = 0xFF };
    memset(params->address, BYTE_ERASED, params->count);
#else
    (void)params;
#endif
}

/*!
Erase a flash sector, disabling interrupts if required.
*/
int32_t FlashKlv::erase_sector(size_t sector, std::span<uint8_t>& flash_bank_memory_slice) // NOLINT(readability-non-const-parameter)
{
    //!!if (flash_bank_memory_slice == nullptr) {
    //    return ERROR_INVALID_FLASH_BANK_PTR;
    //}

    bool already_erased = true;
    enum { BYTE_ERASED = 0xFF };
    for (size_t ii = 0; ii < SECTOR_SIZE; ++ii) {
        if (flash_bank_memory_slice[sector*SECTOR_SIZE + ii] != BYTE_ERASED) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            already_erased = false;
            break;
        }
    }
    if (already_erased) {
        return OK_SECTOR_ALREADY_ERASED;
    }

#if defined(FRAMEWORK_ARDUINO_RPI_PICO)

#if defined(FRAMEWORK_USE_FREERTOS) || defined(PIO_FRAMEWORK_ARDUINO_ENABLE_FREERTOS)
    taskENTER_CRITICAL(); // taskENTER_CRITICAL disables interrupts. This also means context switches are prevented.
    flash_range_erase(reinterpret_cast<uint32_t>(&flash_bank_memory_slice[SECTOR_SIZE * sector]) - XIP_BASE, SECTOR_SIZE);
    taskEXIT_CRITICAL();
#else
    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(reinterpret_cast<uint32_t>(&flash_bank_memory_slice[SECTOR_SIZE * sector]) - XIP_BASE, SECTOR_SIZE);
    restore_interrupts(interrupts);
#endif

#elif defined(FRAMEWORK_RPI_PICO)

#if defined(FRAMEWORK_USE_FREERTOS)
    taskENTER_CRITICAL(); // taskENTER_CRITICAL disables interrupts. This also means context switches are prevented.
    flash_range_erase(reinterpret_cast<uint32_t>(&flash_bank_memory_slice[SECTOR_SIZE * sector]) - XIP_BASE, SECTOR_SIZE);
    taskEXIT_CRITICAL();
#else
    erase_params_t params = { .address = &flash_bank_memory_slice[SECTOR_SIZE * sector], .count = SECTOR_SIZE }; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic,misc-const-correctness)
    const int err = flash_safe_execute(call_flash_range_erase, &params, UINT32_MAX);
    hard_assert(err == PICO_OK);
#endif

#elif defined(FRAMEWORK_TEST)

    erase_params_t params = { .address = &flash_bank_memory_slice[SECTOR_SIZE * sector], .count = SECTOR_SIZE }; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic,misc-const-correctness)
    flash_safe_execute(call_flash_range_erase, &params, UINT32_MAX);

#else

    // FRAMEWORK not supported

#endif // FRAMEWORK
    return OK;
}

/*!
This function is called via the `flash_safe_execute()` wrapper function
*/
void FlashKlv::call_flash_range_program(void* param)
{
    const program_params_t* params = reinterpret_cast<program_params_t*>(param); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
#if defined(FRAMEWORK_RPI_PICO)
    flash_range_program(reinterpret_cast<uint32_t>(params->address) - XIP_BASE, params->data, PAGE_SIZE); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
#elif defined(FRAMEWORK_TEST)
    memcpy(params->address, params->data, PAGE_SIZE);
#else
    (void)params;
#endif // FRAMEWORK
}

/*!
Write a flash page, disabling interrupts if required.
*/
void FlashKlv::flash_write_page(size_t page_index, std::span<uint8_t>& flash_memory_slice) // NOLINT(readability-convert-member-functions-to-static)
{
#if defined(FRAMEWORK_ARDUINO_RPI_PICO)

#if defined(FRAMEWORK_USE_FREERTOS) || defined(PIO_FRAMEWORK_ARDUINO_ENABLE_FREERTOS)
    taskENTER_CRITICAL(); // taskENTER_CRITICAL disables interrupts. This also means context switches are prevented.
    flash_range_program(reinterpret_cast<uint32_t>(&flash_memory_slice[page_index*PAGE_SIZE]) - XIP_BASE, &_page_cache[0], FLASH_PAGE_SIZE);
    taskEXIT_CRITICAL();
#else
    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_program(reinterpret_cast<uint32_t>(&flash_memory_slice[page_index*PAGE_SIZE]) - XIP_BASE, &_page_cache[0], FLASH_PAGE_SIZE);
    restore_interrupts(interrupts);
#endif

#elif defined(FRAMEWORK_RPI_PICO)

#if defined(FRAMEWORK_USE_FREERTOS)
    taskENTER_CRITICAL(); // taskENTER_CRITICAL disables interrupts. This also means context switches are prevented.
    flash_range_program(reinterpret_cast<uint32_t>(&flash_memory_slice[page_index*PAGE_SIZE]) - XIP_BASE, &_page_cache[0], FLASH_PAGE_SIZE);
    taskEXIT_CRITICAL();
#else
    program_params_t params = { .address = &flash_memory_slice[page_index*PAGE_SIZE], .data = &_page_cache[0] };
    const int err = flash_safe_execute(call_flash_range_program, &params, UINT32_MAX);
    hard_assert(err == PICO_OK);
#endif

#elif defined(FRAMEWORK_TEST)

    //program_params_t params = { .address = &flash_memory_slice[page_index*PAGE_SIZE], .data = &_page_cache[0] };
    program_params_t params = { .address = flash_memory_slice.data() + page_index*PAGE_SIZE, .data = &_page_cache[0] };
    flash_safe_execute(call_flash_range_program, &params, UINT32_MAX);
    //memcpy(params->address, params->data, PAGE_SIZE);
    /*std::copy(flash_memory_slice.begin() + page_index*PAGE_SIZE,
          flash_memory_slice.begin() + page_index*PAGE_SIZE + PAGE_SIZE,
          &_page_cache[0]);*/

    //memcpy(flash_memory_slice.data() + page_index*PAGE_SIZE, &_page_cache[0], PAGE_SIZE);

#else

    //FRAMEWORK not supported

#endif // FRAMEWORK
}
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
