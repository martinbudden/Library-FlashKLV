#include "FlashKLV.h"
#include <cassert>
#include <cstring>


#if defined(FRAMEWORK_RPI_PICO)
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <pico/flash.h>
#else
int flash_safe_execute(void (*fn)(void*), void* param, uint32_t timeout_ms)
{
    (void)timeout_ms;
    fn(param);
    return 0;
}
#endif


/*!
If bankCount == 1 then we have one bank of flash at 
address: flashMemoryPtr, size: SECTOR_SIZE*sectorsPerBank

If bankCount == 2 then we have
BANK_A address: flashMemoryPtr, size: SECTOR_SIZE*sectorsPerBank
BANK_B address: flashMemoryPtr + SECTOR_SIZE*sectorsPerBank, size: SECTOR_SIZE*sectorsPerBank
*/
FlashKLV::FlashKLV(uint8_t* flashMemoryPtr, size_t sectorsPerBank, size_t bankCount) :
    _flashBaseMemoryPtr(flashMemoryPtr),
    _currentBankMemoryPtr(_flashBaseMemoryPtr),
    _bankMemorySize(sectorsPerBank * SECTOR_SIZE),
    _bankSectorCount(sectorsPerBank)
{
    assert(bankCount == 1 || bankCount == 2);

    uint8_t* BANK_A_PTR = flashMemoryPtr;
    uint8_t* BANK_B_PTR = flashMemoryPtr + _bankMemorySize; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (bankCount == 2) {
        // if the BANK_B has data in it, then that should be the current bank
        if (memcmp(BANK_B_PTR, &BANK_HEADER[0], sizeof(BANK_HEADER)) == 0) {
            _currentBankMemoryPtr = BANK_B_PTR;
            _otherBankMemoryPtr = BANK_A_PTR;
        } else {
            _currentBankMemoryPtr = BANK_A_PTR;
            _otherBankMemoryPtr = BANK_B_PTR;
        }
    }
#if defined(FRAMEWORK_RPI_PICO)
    static_assert(FLASH_SECTOR_SIZE == SECTOR_SIZE);
    static_assert(FLASH_PAGE_SIZE == PAGE_SIZE);
#endif
}

// For RP2040 PICO_FLASH_SIZE_BYTES is 2MB or 2097152 bytes.
// For RP2350 it is 4MB
FlashKLV::FlashKLV(size_t sectorsPerBank, size_t bankCount) : // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
#if defined(FRAMEWORK_RPI_PICO)
    FlashKLV(reinterpret_cast<uint8_t*>(XIP_BASE + PICO_FLASH_SIZE_BYTES - SECTOR_SIZE*sectorsPerBank*bankCount), sectorsPerBank, bankCount)
#else
    FlashKLV(nullptr, sectorsPerBank, bankCount)
#endif
{
}

FlashKLV::FlashKLV(uint8_t* flashMemoryPtr, size_t sectorsPerBank) :
    FlashKLV(flashMemoryPtr, sectorsPerBank, ONE_BANK) {}

FlashKLV::FlashKLV(size_t sectorsPerBank) :
    FlashKLV(sectorsPerBank, ONE_BANK) {}

/*!
Flash can be overwritten if bits are only flipped from 1 to 0, never from 0 to 1
*/
bool FlashKLV::overwriteable(uint8_t flash, uint8_t value)
{
    for (uint8_t bit = 0x80; bit !=0; bit >>= 1U) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        if (((flash & bit) == 0) && ((value & bit) == bit)) {
            return false;
        }
    }
    return true;
}

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
bool FlashKLV::overwriteable(const uint8_t* flashPtr, const uint8_t* valuePtr, uint16_t length)
{
    for (size_t ii = 0; ii < length; ++ ii) {
        const uint8_t flash = *flashPtr++;
        const uint8_t value = *valuePtr++;

        for (uint8_t bit = 0x80; bit !=0; bit >>= 1U) { // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            if (((flash & bit) == 0) && ((value & bit) == bit)) {
                return false;
            }
        }
    }
    return true;
}

uint16_t FlashKLV::getRecordKey(size_t pos, const uint8_t* flashMemoryPtr)
{
    const uint8_t key8 = flashMemoryPtr[pos];
    if ((key8 & UNDELETED_BIT) == 0) {
        return RECORD_KEY_DELETED;
    }
    if ((key8 & KL16_BIT) == 0) {
        enum : uint8_t { KEY8_MASK = 0x3F };
        return key8 & KEY8_MASK;
    }
    // note byte order is reversed, this is done because and empty record is denoted by setting the high bit of the high byte to zero.
    uint16_t key16 = key8;
    key16 = (key16 << 8U) | flashMemoryPtr[pos + 1]; // NOLINT(cppcoreguidelines-avoid-magic-numbers,hicpp-signed-bitwise,readability-magic-numbers)
    if (key16 == RECORD_KEY_EMPTY) {
        return RECORD_KEY_EMPTY;
    }
    enum : uint16_t { KEY16_MASK = 0x3FFF };
    return key16 & KEY16_MASK;
}

bool FlashKLV::isRecordEmpty(size_t pos, const uint8_t* flashMemoryPtr)
{
    const uint8_t octet = flashMemoryPtr[pos];
    return (octet == RECORD_EMPTY) ? true : false; // NOLINT(readability-simplify-boolean-expr)
}

uint16_t FlashKLV::getRecordLength(size_t pos, const uint8_t* flashMemoryPtr)
{
    if ((flashMemoryPtr[pos] & KL16_BIT) == 0) {
        return flashMemoryPtr[pos + sizeof(kl8_t::key)];
    }
    const uint16_t lengthL = flashMemoryPtr[pos + sizeof(kl16_t::key)];
    const uint16_t lengthH = flashMemoryPtr[pos + sizeof(kl16_t::key) + 1];
    return lengthL | (lengthH << 8U); // NOLINT(cppcoreguidelines-avoid-magic-numbers,hicpp-signed-bitwise,readability-magic-numbers)
}

uint16_t FlashKLV::getRecordPositionIncrement(size_t pos, const uint8_t* flashMemoryPtr)
{
    const size_t offset = ((flashMemoryPtr[pos] & KL16_BIT) == 0) ? sizeof(kl8_t) : sizeof(kl16_t);
    const uint16_t ret = getRecordLength(pos, flashMemoryPtr) + static_cast<uint16_t>(offset);
    return ret;
}

const uint8_t* FlashKLV::getRecordValuePtr(size_t pos, const uint8_t* flashMemoryPtr)
{
    if ((flashMemoryPtr[pos] & KL16_BIT) == 0) {
        return &flashMemoryPtr[pos + sizeof(kl8_t)];
    }
    return &flashMemoryPtr[pos + sizeof(kl16_t)];
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

bool FlashKLV::isEmpty(uint16_t flashRecordKey)
{
    return flashRecordKey == RECORD_KEY_EMPTY;
}

size_t FlashKLV::bytesFree() const
{
    size_t pos = 0;
    uint16_t flashRecordKey = getRecordKey(pos);
    while (!isEmpty(flashRecordKey)) {
        pos += getRecordPositionIncrement(pos);
        if (pos >= _bankMemorySize) {
            return 0;
        }
        flashRecordKey = getRecordKey(pos);
    }
    return _bankMemorySize - pos;
}

int32_t FlashKLV::remove(uint16_t key, uint8_t* flashMemoryPtr)
{
    if (!keyOK(key)) {
        return ERROR_INVALID_KEY;
    }

    size_t pos = 0;
    uint16_t flashRecordKey = getRecordKey(pos, flashMemoryPtr);
    while (!isEmpty(flashRecordKey)) {
        if (flashRecordKey == key) {
            flashMarkRecordAsDeleted(pos, flashMemoryPtr);
            return OK;
        }
        pos += getRecordPositionIncrement(pos);
        if (pos >= _bankMemorySize) {
            return ERROR_NOT_FOUND;
        }
        flashRecordKey = getRecordKey(pos);
    }
    return ERROR_NOT_FOUND;
}

/*!
Find the record with the specified key.
*/
FlashKLV::klv_t FlashKLV::find(uint16_t key) const
{
    if (!keyOK(key)) {
        return klv_t {.key = NOT_FOUND, .length = 0, .valuePtr = nullptr};
    }

    // Walk the flash until skipping over deleted records and records with a different key.
    size_t pos = 0;
    uint16_t flashRecordKey = getRecordKey(pos);
    while (!isEmpty(flashRecordKey)) {
        if (flashRecordKey == key) {
            return klv_t {.key = key, .length = getRecordLength(pos), .valuePtr = getRecordValuePtr(pos)};
        }
        pos += getRecordPositionIncrement(pos);
        if (pos >= _bankMemorySize) {
            return klv_t {.key = NOT_FOUND, .length = 0, .valuePtr = nullptr};
        }
        flashRecordKey = getRecordKey(pos);
    }
    return klv_t {.key = NOT_FOUND, .length = 0, .valuePtr = nullptr};
}

/*!
Find the next undeleted record with any key, starting at pos.

Find always searches the current bank.
*/
FlashKLV::klv_t FlashKLV::findNext(size_t pos) const
{
    // skip over the record at pos
    uint16_t flashRecordKey = getRecordKey(pos);
    if (isEmpty(flashRecordKey)) {
        return klv_t {.key = NOT_FOUND, .length = 0, .valuePtr = nullptr};
    }
    pos += getRecordPositionIncrement(pos);

    // now walk the flash to find the next undeleted record
    flashRecordKey = getRecordKey(pos);
    while (!isEmpty(flashRecordKey)) {
        if (flashRecordKey != RECORD_KEY_DELETED) {
            return klv_t {.key = flashRecordKey, .length = getRecordLength(pos), .valuePtr = getRecordValuePtr(pos)};
        }
        pos += getRecordPositionIncrement(pos);
        if (pos >= _bankMemorySize) {
            return klv_t {.key = NOT_FOUND, .length = 0, .valuePtr = nullptr};
        }
        flashRecordKey = getRecordKey(pos);
    }
    return klv_t {.key = NOT_FOUND, .length = 0, .valuePtr = nullptr};
}

int32_t FlashKLV::copyRecordsToOtherBank()
{
    if (_otherBankMemoryPtr == nullptr) {
        return ERROR_OTHER_BANK_NOT_INITIALIZED;
    }

    if (!isOtherBankErased()) {
        return ERROR_OTHER_BANK_NOT_ERASED;
    }

    size_t writePos = sizeof(BANK_HEADER);
    flashReadPage(0, _otherBankMemoryPtr);
    memcpy(&_pageCache[0], &BANK_HEADER[0], sizeof(BANK_HEADER));
    flashWritePage(0, _otherBankMemoryPtr);

    size_t pos = 0;
    uint16_t flashRecordKey = getRecordKey(pos);
    if (isEmpty(flashRecordKey)) {
        return OK_NOTHING_TO_COPY;
    }

    while (!isEmpty(flashRecordKey)) {
        if (flashRecordKey != RECORD_KEY_DELETED && flashRecordKey != RECORD_KEY_BANK_HEADER) {
            // not a deleted record, so write it to the other bank
            flashDeleteAndWrite(NO_DELETE, writePos, flashRecordKey, getRecordLength(pos), getRecordValuePtr(pos), _otherBankMemoryPtr);
            writePos += getRecordPositionIncrement(pos);
        }
        pos += getRecordPositionIncrement(pos);
        if (pos >= _bankMemorySize) {
            return ERROR_FLASH_FULL;
        }
        flashRecordKey = getRecordKey(pos);
    }
    return writePos > sizeof(BANK_HEADER) ? OK : OK_NO_RECORDS_COPIED;
}

int32_t FlashKLV::copyRecordsToOtherBankAndSwapBanks()
{
    const int32_t err = copyRecordsToOtherBank();
    if (err < 0) {
        return err;
    }
    swapBanks();
    // erase the first sector of the other bank, so that it will not be chosen as the current bank on the next FlashKLV construction
    eraseSector(0, _otherBankMemoryPtr);

    return err;
}

/*!
Read the record with the given key.

Read always reads the current bank.
*/
int32_t FlashKLV::read(void* value, size_t size, uint16_t key) const
{
    if (!keyOK(key)) {
        return ERROR_INVALID_KEY;
    }

    const FlashKLV::klv_t klv = find(key);
    if (klv.key == NOT_FOUND) {
        return ERROR_NOT_FOUND;
    }
    if (klv.length > size) {
        memcpy(value, klv.valuePtr, size);
        return ERROR_RECORD_TOO_LARGE;
    }
    memcpy(value, klv.valuePtr, klv.length);
    if (klv.length < size) {
        return ERROR_RECORD_TOO_SMALL;
    }
    return OK;
}

/*
Write a record

May write to either the current bank or the other bank, as defined by flashMemoryPtr.
*/
int32_t FlashKLV::write(uint16_t key, uint16_t length, const uint8_t* valuePtr, uint8_t* flashMemoryPtr)
{
    if (!keyOK(key)) {
        return ERROR_INVALID_KEY;
    }

    size_t pos = 0;
    size_t deletePos = NO_DELETE;

    // look for an empty position to write the new record
    uint16_t flashRecordKey = getRecordKey(pos);
    while (!isEmpty(flashRecordKey)) {
        if (flashRecordKey == key) {
            // there is already a record of this key, so first check if can be reused
            if (getRecordLength(pos) == length) {
                // new record is same length as old, so check if it has changed
                if (!memcmp(getRecordValuePtr(pos), valuePtr, length)) {
                    // record has not changed, so no need to write it
                    return OK_NO_NEED_TO_WRITE;
                }
                // record has changed, so check if it can the be overwritten, that is bits are only flipped from 1 to 0, never from 0 to 1
                if (overwriteable(getRecordValuePtr(pos), valuePtr, length)) {
                    // just overwrite the value: key and length are unchanged
                    flashWrite(pos + (((flashMemoryPtr[pos] & KL16_BIT) == 0) ? sizeof(kl8_t) : sizeof(kl16_t)), length, valuePtr, flashMemoryPtr); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    return OK_OVERWRITTEN;
                }
            }
            // record has changed, so save the old one's position for later deletion and continue looking for empty position
            deletePos = pos;
        }

        pos += getRecordPositionIncrement(pos);
        if (pos + length > _bankMemorySize) {
            // not enough space for the new record
            return ERROR_FLASH_FULL;
        }
        flashRecordKey = getRecordKey(pos);
    }

    // we've found an empty position, so write the record there
    flashDeleteAndWrite(deletePos, pos, key, length, valuePtr, flashMemoryPtr);
    return OK;
}

// This function is called via `flash_safe_execute()`
void FlashKLV::call_flash_range_erase(void* param)
{
    const erase_params_t* params = reinterpret_cast<erase_params_t*>(param); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
#if defined(FRAMEWORK_RPI_PICO)
    flash_range_erase(reinterpret_cast<uint32_t>(params->address) - XIP_BASE, params->count);
#elif defined(FRAMEWORK_TEST)
    memset(params->address, 0xFF, params->count);
#else
    (void)params;
#endif
}

int32_t FlashKLV::eraseSector(size_t sector, uint8_t* flashBankMemoryPtr) // NOLINT(readability-non-const-parameter)
{
    if (flashBankMemoryPtr == nullptr) {
        return ERROR_INVALID_FLASH_BANK_PTR;
    }

    bool alreadyErased = true;
    enum { BYTE_ERASED = 0xFF };
    for (size_t ii = 0; ii < SECTOR_SIZE; ++ii) {
        if (*(flashBankMemoryPtr + sector*SECTOR_SIZE + ii) != BYTE_ERASED) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            alreadyErased = false;
        }
    }
    if (alreadyErased) {
        return OK_SECTOR_ALREADY_ERASED;
    }
    erase_params_t params = { .address = &flashBankMemoryPtr[SECTOR_SIZE * sector], .count = SECTOR_SIZE }; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic,misc-const-correctness)
#if defined(FRAMEWORK_RPI_PICO)
    // Flash is "execute in place" and so will be in use when any code that is stored in flash runs, e.g. an interrupt handler
    // or code running on a different core.
    // Calling flash_range_erase or flash_range_program at the same time as flash is running code would cause a crash.
    // flash_safe_execute disables interrupts and tries to cooperate with the other core to ensure flash is not in use
    // See the documentation for flash_safe_execute and its assumptions and limitations
    const int err = flash_safe_execute(call_flash_range_erase, &params, UINT32_MAX);
    hard_assert(err == PICO_OK);
#elif defined(FRAMEWORK_ARDUINO_RPI_PICO)
#ifndef __FREERTOS
    noInterrupts();
#endif
    rp2040.idleOtherCore();
    flash_range_erase(reinterpret_cast<uint32_t>(params.address) - XIP_BASE, params.count);
    rp2040.resumeOtherCore();
#ifndef __FREERTOS
    interrupts();
#endif
#else
    flash_safe_execute(call_flash_range_erase, &params, UINT32_MAX);
#endif // FRAMEWORK
    return OK;
}

bool FlashKLV::isSectorErased(size_t sector, const uint8_t* flashBankMemoryPtr)
{
    enum { BYTE_ERASED = 0xFF };

    const uint8_t* memoryPtr = flashBankMemoryPtr + sector*SECTOR_SIZE; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (size_t ii = 0; ii < SECTOR_SIZE; ++ii) {
        if (*(memoryPtr + ii) != BYTE_ERASED) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return false;
        }
    }
    return true;
}

bool FlashKLV::isBankErased(const uint8_t* flashBankMemoryPtr) const
{
    enum { BYTE_ERASED = 0xFF };
    for (size_t ii = 0; ii < _bankMemorySize; ++ii) {
        if (*(flashBankMemoryPtr + ii) != BYTE_ERASED) { // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return false;
        }
    }
    return true;
}

int32_t FlashKLV::eraseBank(uint8_t* flashBankMemoryPtr) // NOLINT(readability-make-member-function-const)
{
    for (uint32_t ii = 0; ii < _bankSectorCount; ++ii) {
        eraseSector(ii, flashBankMemoryPtr);
    }
    return OK;
}

void FlashKLV::flashMarkRecordAsDeleted(size_t pos, uint8_t* flashMemoryPtr)
{
    const size_t pageIndex = pos / PAGE_SIZE;
    flashReadPage(pageIndex, flashMemoryPtr);

    const size_t pageOffset = pos - pageIndex*PAGE_SIZE;
    _pageCache[pageOffset] &= DELETED_MASK;

    flashWritePage(pageIndex, flashMemoryPtr);
}

void FlashKLV::flashReadPage(size_t pageIndex, const uint8_t* flashMemoryPtr)
{
    memcpy(&_pageCache[0], &flashMemoryPtr[pageIndex*PAGE_SIZE], PAGE_SIZE);
}

// This function is called via `flash_safe_execute()`
void FlashKLV::call_flash_range_program(void* param)
{
    const program_params_t* params = reinterpret_cast<program_params_t*>(param); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
#if defined(FRAMEWORK_RPI_PICO)
    flash_range_program(reinterpret_cast<uint32_t>(params->address) - XIP_BASE, params->data, PAGE_SIZE); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
#elif defined(FRAMEWORK_TEST)
    memcpy(params->address, params->data, PAGE_SIZE);
#else
    (void)params;
#endif
}

void FlashKLV::flashWritePage(size_t pageIndex, uint8_t* flashMemoryPtr) // NOLINT(readability-convert-member-functions-to-static)
{
    program_params_t params = { .address = &flashMemoryPtr[pageIndex*PAGE_SIZE], .data = &_pageCache[0] };
#if defined(FRAMEWORK_RPI_PICO)
    const int err = flash_safe_execute(call_flash_range_program, &params, UINT32_MAX);
    hard_assert(err == PICO_OK);
#elif defined(FRAMEWORK_ARDUINO_RPI_PICO)
#ifndef __FREERTOS
    noInterrupts();
#endif
    rp2040.idleOtherCore();
    flash_range_program(reinterpret_cast<uint32_t>(params.address) - XIP_BASE, params.data, PAGE_SIZE); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    rp2040.resumeOtherCore();
#ifndef __FREERTOS
    interrupts();
#endif
#else
    flash_safe_execute(call_flash_range_program, &params, UINT32_MAX);
#endif
}

void FlashKLV::flashWrite(size_t pos, uint16_t length, const uint8_t* valuePtr, uint8_t* flashMemoryPtr)
{
    while (length != 0) {
        const size_t pageIndex = pos / PAGE_SIZE;
        flashReadPage(pageIndex, flashMemoryPtr);

        const size_t pageOffset = pos - pageIndex*PAGE_SIZE;
        const size_t bytesToEndOfPage = PAGE_SIZE - pageOffset;

        if (length <= bytesToEndOfPage) {
            // everything can fit on the current page, so copy it and return
            memcpy(&_pageCache[pageOffset], valuePtr, length);
            flashWritePage(pageIndex, flashMemoryPtr);
            return;
        }
        // copy what we have room for and then loop round for the next chunk
        memcpy(&_pageCache[pageOffset], valuePtr, bytesToEndOfPage);
        flashWritePage(pageIndex, flashMemoryPtr);

        pos += bytesToEndOfPage;
        valuePtr += bytesToEndOfPage; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        length -= static_cast<uint16_t>(bytesToEndOfPage);
    }
}

void FlashKLV::flashDeleteAndWrite(size_t deletePos, size_t pos, uint16_t key, uint16_t length, const uint8_t* valuePtr, uint8_t* flashMemoryPtr)
{
    size_t pageIndex = pos / PAGE_SIZE;

    const size_t deletePageIndex = (deletePos == NO_DELETE) ? NO_DELETE : deletePos / PAGE_SIZE;
    if (deletePos != NO_DELETE && deletePageIndex != pageIndex) {
        // there is a record to mark as deleted, and it is in a different page than the record being written
        flashMarkRecordAsDeleted(deletePos, flashMemoryPtr);
    }

    flashReadPage(pageIndex, flashMemoryPtr);
    if (deletePageIndex == pageIndex) {
        // if there is a deleted record, then mark it as deleted
        // note that deletePos < pos, so there is no danger of this being overwritten
        _pageCache[deletePos - pageIndex*PAGE_SIZE] &= DELETED_MASK;
    }

    size_t pageOffset = pos - pageIndex*PAGE_SIZE;
    size_t bytesToEndOfPage = PAGE_SIZE - pageOffset;

    if (key <= KEY8_MAX) {
        key |= UNDELETED_BIT;
    } else {
        // reverse key byte order, this is done because and empty record is denoted by setting the high byte to zero.
        key = (key << 8U) | (key >> 8U) | UNDELETED_BIT | KL16_BIT; // NOLINT(cppcoreguidelines-avoid-magic-numbers,hicpp-signed-bitwise,readability-magic-numbers)
    }
    const kl8_t keyLength8 = { .key = static_cast<uint8_t>(key), .length = static_cast<uint8_t>(length) };
    const kl16_t keyLength16 = { .key = key, .length = length };
    const auto* keyLengthPtr = ((key&KL16_BIT) == 0) ? reinterpret_cast<const uint8_t*>(&keyLength8) : reinterpret_cast<const uint8_t*>(&keyLength16); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    const size_t sizeofKeyLength = ((key&KL16_BIT) == 0) ? sizeof(kl8_t) : sizeof(kl16_t);

    // update the pageCache with key and value, dealing with the case where key and value can span two pages
    if (sizeofKeyLength >= bytesToEndOfPage) {
        // key and value fill this page
        // so copy up to the end of this page
        memcpy(&_pageCache[pageOffset], keyLengthPtr, bytesToEndOfPage);
        flashWritePage(pageIndex++, flashMemoryPtr);
        flashReadPage(pageIndex, flashMemoryPtr);
        if (sizeofKeyLength > bytesToEndOfPage) {
            // key and value spill onto next page
            // so copy the rest onto the next page
            memcpy(&_pageCache[0], keyLengthPtr + bytesToEndOfPage, sizeofKeyLength - bytesToEndOfPage);
        }
    } else {
        // key and value are contained within this page
        memcpy(&_pageCache[pageOffset], keyLengthPtr, sizeofKeyLength);
    }

    pos += sizeofKeyLength;
    pageOffset = pos - pageIndex*PAGE_SIZE;
    bytesToEndOfPage = PAGE_SIZE - pageOffset;
    if (length <= bytesToEndOfPage) {
        // the whole of the rest of the record fits in this page
        memcpy(&_pageCache[pageOffset], valuePtr, length);
        flashWritePage(pageIndex, flashMemoryPtr);
        return;
    }

    // copy the part of the record that fits on this page and write it
    memcpy(&_pageCache[pageOffset], valuePtr, bytesToEndOfPage);
    flashWritePage(pageIndex, flashMemoryPtr);

    // deal with the rest of the record
    pos += bytesToEndOfPage;
    valuePtr += bytesToEndOfPage; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    length -= static_cast<uint16_t>(bytesToEndOfPage);
    flashWrite(pos, length, valuePtr, flashMemoryPtr);
}
