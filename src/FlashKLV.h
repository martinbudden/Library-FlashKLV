#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

/*!
Flash Key Length Value (KLV) storage.
*/
class FlashKLV {
public:
    enum { ONE_BANK = 1, TWO_BANKS = 2 };
    enum {
        OK = 0, OK_NO_NEED_TO_WRITE = 1, OK_OVERWRITTEN = 2, OK_NOTHING_TO_COPY = 3, OK_SECTOR_ALREADY_ERASED = 4, OK_NO_RECORDS_COPIED = 5,
        ERROR_FLASH_FULL = -1, ERROR_NOT_FOUND = -2, ERROR_INVALID_KEY = -3,
        ERROR_RECORD_TOO_LARGE = -4, ERROR_RECORD_TOO_SMALL = -5,
        ERROR_OTHER_BANK_NOT_INITIALIZED = -6, ERROR_OTHER_BANK_NOT_ERASED = -7,
        ERROR_INVALID_FLASH_BANK_PTR = -8
    };
    enum { NOT_FOUND = 0 };
    enum { RECORD_KEY_EMPTY = 0xFFFF, RECORD_KEY_BANK_HEADER = 0x3FFE, RECORD_KEY_DELETED = 0, RECORD_EMPTY = 0xFF };
    enum : uint8_t { KEY8_MIN = 0x01, KEY8_MAX = 0x3F };
    enum : uint16_t { KEY16_MIN = 0x0100, KEY16_MAX = 0x3FFD };
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
        const uint8_t* valuePtr;
    };
    struct kl8_t { // key and length for 8-bit KLV record
        uint8_t key;
        uint8_t length;
    };
    struct kl16_t { // key and length for 16-bit KLV record
        uint16_t key;
        uint16_t length;
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
    FlashKLV(uint8_t* flashMemoryPtr, size_t sectorsPerBank, size_t bankCount);
    FlashKLV(uint8_t* flashMemoryPtr, size_t sectorsPerBank);
    FlashKLV(size_t sectorsPerBank, size_t bankCount);
    explicit FlashKLV(size_t sectorsPerBank);
    ~FlashKLV() = default;
public:
    static bool keyOK(uint16_t key) { return ((key >= KEY8_MIN && key <= KEY8_MAX) || (key >= KEY16_MIN && key <= KEY16_MAX)); }

    size_t bytesFree() const;
    size_t memorySize() const { return _bankMemorySize; }

    klv_t find(uint16_t key) const;
    klv_t findNext(size_t pos) const;
    int32_t copyRecordsToOtherBankAndSwapBanks();
    int32_t read(void* value, size_t size,  uint16_t key) const;

    int32_t remove(uint16_t key) { return remove(key, _currentBankMemoryPtr); }
    int32_t write(uint16_t key, uint16_t length, const uint8_t* valuePtr) { return write(key, length, valuePtr, _currentBankMemoryPtr); }
    int32_t write(uint16_t key, uint16_t length, const void* valuePtr) { return write(key, length, static_cast<const uint8_t*>(valuePtr)); }
    int32_t write(const klv_t& klv) { return write(klv.key, klv.length, klv.valuePtr); }

    bool isCurrentBankErased() const  { return isBankErased(_currentBankMemoryPtr); }
    bool isOtherBankErased() const  { return isBankErased(_otherBankMemoryPtr); }
    int32_t  eraseCurrentBank() { return eraseBank(_currentBankMemoryPtr); }
    int32_t  eraseOtherBank() { return eraseBank(_otherBankMemoryPtr); }
    size_t getBankSectorCount() const { return _bankSectorCount; }
    bool isSectorErasedCurrentBank(size_t sector) const  { return isSectorErased(sector, _currentBankMemoryPtr); }
    bool isSectorErasedOtherBank(size_t sector) const  { return isSectorErased(sector, _otherBankMemoryPtr); }
protected:
    static bool isSectorErased(size_t sector, const uint8_t* flashBankMemoryPtr);
    bool isBankErased(const uint8_t* flashBankMemoryPtr) const;
    int32_t eraseBank(uint8_t* flashBankMemoryPtr);
    static int32_t eraseSector(size_t sector, uint8_t* flashBankMemoryPtr);

    int32_t remove(uint16_t key, uint8_t* flashMemoryPtr);
    int32_t write(uint16_t key, uint16_t length, const uint8_t* valuePtr, uint8_t* flashMemoryPtr);
    void flashMarkRecordAsDeleted(size_t pos, uint8_t* flashMemoryPtr);
    void flashWrite(size_t pos, uint16_t length, const uint8_t* valuePtr, uint8_t* flashMemoryPtr);
    void flashDeleteAndWrite(size_t deletePos, size_t pos, uint16_t key, uint16_t length, const uint8_t* valuePtr, uint8_t* flashMemoryPtr);
    void flashReadPage(size_t pageIndex, const uint8_t* flashMemoryPtr);
    void flashWritePage(size_t pageIndex, uint8_t* flashMemoryPtr);

    static bool isRecordEmpty(size_t pos, const uint8_t* flashMemoryPtr);
    static uint16_t getRecordKey(size_t pos, const uint8_t* flashMemoryPtr);
    static uint16_t getRecordLength(size_t pos, const uint8_t* flashMemoryPtr);
    static uint16_t getRecordPositionIncrement(size_t pos, const uint8_t* flashMemoryPtr);
    static const uint8_t* getRecordValuePtr(size_t pos, const uint8_t* flashMemoryPtr);

    static void call_flash_range_erase(void* param);
    static void call_flash_range_program(void* param);
public:
// internal, exposed for testing
    static bool overwriteable(uint8_t flash, uint8_t value);
    static bool overwriteable(const uint8_t* flashPtr, const uint8_t* valuePtr, uint16_t length);
    bool isRecordEmpty(size_t pos) const { return isRecordEmpty(pos, _currentBankMemoryPtr); }
    static bool isEmpty(uint16_t flashRecordKey);
    uint16_t getRecordKey(size_t pos) const { return getRecordKey(pos, _currentBankMemoryPtr); }
    uint16_t getRecordLength(size_t pos) const { return getRecordLength(pos, _currentBankMemoryPtr); }
    uint16_t getRecordPositionIncrement(size_t pos) const { return getRecordPositionIncrement(pos, _currentBankMemoryPtr); }
    const uint8_t* getRecordValuePtr(size_t pos) const { return getRecordValuePtr(pos, _currentBankMemoryPtr); }
    int32_t copyRecordsToOtherBank();
    void swapBanks() { uint8_t* otherBankMemoryPtr = _otherBankMemoryPtr; _otherBankMemoryPtr = _currentBankMemoryPtr; _currentBankMemoryPtr = otherBankMemoryPtr; }
// for testing
    const uint8_t* flashPos(size_t pos) { return _currentBankMemoryPtr + pos; } //!< for testing
    uint8_t flashPeek(size_t pos) { return _currentBankMemoryPtr[pos]; } //!< for testing
    uint8_t flashPeekOther(size_t pos) { return _otherBankMemoryPtr[pos]; } //!< for testing
    const uint8_t* getCurrentBankMemoryPtr() const { return _currentBankMemoryPtr; }
    const uint8_t* getOtherBankMemoryPtr() const { return _otherBankMemoryPtr; }
    int32_t eraseSector(size_t sector) { return eraseSector(sector, _currentBankMemoryPtr); }
protected:
    uint8_t* _flashBaseMemoryPtr;
    uint8_t* _currentBankMemoryPtr;
    uint8_t* _otherBankMemoryPtr {};
    size_t _bankMemorySize; //!< the size of each memory bank
    size_t _bankSectorCount; //!< the number of sectors in each memory bank
    std::array<uint8_t, PAGE_SIZE> _pageCache {};
    static constexpr std::array<uint8_t, 8> BANK_HEADER =  { 0xFF, 0xFE, 0x04, 0x00, 0xF1, 0xF2, 0xF3, 0xF4 };
};
