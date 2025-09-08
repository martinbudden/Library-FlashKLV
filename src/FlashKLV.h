#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

/*!
Flash Key Length Value (KLV) storage.

8-bit form:
k l value
k in range [1, 63] ie [0x01, 0x3F] ie [[0b0000'0001, 0b0011'1111]
16-bit form
kk ll value
kk in range [0x01000,0x3fff], 

*/
class FlashKLV {
public:
#if defined(FRAMEWORK_TEST)
    enum : size_t { SECTOR_SIZE = 512 };  //! Minimum erasable amount, bytes.
    enum { PAGE_SIZE = 8 }; //! Minimum writable amount, bytes.
    FlashKLV(uint8_t* flashMemory, size_t flashMemorySize);
#else
    enum : size_t { SECTOR_SIZE = 4096 };  //! Minimum erasable amount, bytes.
    enum { PAGE_SIZE = 256 }; //! Minimum writable amount, bytes.
#endif
public:
    enum {
        OK = 0, OK_NO_NEED_TO_WRITE = 1, OK_OVERWRITTEN = 2,
        ERROR_FLASH_FULL = -1, ERROR_NOT_FOUND = -2, ERROR_INVALID_KEY = -3, ERROR_RECORD_TOO_LARGE = -4, ERROR_RECORD_TOO_SMALL = -5
    };
    enum { NOT_FOUND = 0 };
    enum { RECORD_KEY_EMPTY = 0xFFFF, RECORD_EMPTY = 0xFF };
    enum : uint8_t { KEY8_MIN = 0x01, KEY8_MAX = 0x3F };
    enum : uint16_t { KEY16_MIN = 0x0100, KEY16_MAX = 0x3FFF };
protected:
    enum { FLASH_NOT_SET = 0xFF, FLASH_SET = 0x00 }; // unwritten flash is 0xFF
    enum : uint8_t { DELETED_MASK = 0x7F, UNDELETED_BIT = 0x80, KL16_BIT = 0x40 };
    enum { NO_DELETE = 0xFFFF };
public:
    struct klv_t {
        uint16_t key;
        uint16_t length;
        uint8_t* valuePtr;
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
    explicit FlashKLV(size_t flashMemorySize);
    ~FlashKLV() = default;
public:
    static bool keyOK(uint16_t key) { return ((key >= KEY8_MIN && key <= KEY8_MAX) || (key >= KEY16_MIN && key <= KEY16_MAX)); }

    size_t bytesFree() const;
    size_t memorySize() const { return _flashMemorySize; }

    int32_t remove(uint16_t key);
    klv_t find(uint16_t key) const;
    int32_t read(void* value, size_t size,  uint16_t key) const;

    int32_t write(uint16_t key, uint16_t length, const uint8_t* valuePtr);
    int32_t write(uint16_t key, uint16_t length, const void* valuePtr) { return write(key, length, static_cast<const uint8_t*>(valuePtr)); }
    int32_t write(const klv_t& klv) { return write(klv.key, klv.length, klv.valuePtr); }

    void eraseSector(uint32_t sector);
    void eraseAllSectors();
    uint32_t getSectorCount() const { return _sectorCount; }
protected:
    void flashMarkRecordAsDeleted(size_t pos);
    void flashWrite(size_t pos, uint16_t length, const uint8_t* valuePtr);
    void flashDeleteAndWrite(size_t deletePos, size_t pos, uint16_t key, uint16_t length, const uint8_t* valuePtr);
    void flashReadPage(size_t pageIndex);
    void flashWritePage(size_t pageIndex);

    static void call_flash_range_erase(void* param);
    static void call_flash_range_program(void* param);
public:
// internal, exposed for testing
    static bool overwriteable(uint8_t flash, uint8_t value);
    static bool overwriteable(const uint8_t* flashPtr, const uint8_t* valuePtr, uint16_t length);
    bool isRecordEmpty(size_t pos) const;
    static bool isEmpty(uint16_t flashRecordKey);
    uint16_t getRecordKey(size_t pos) const;
    uint16_t getRecordLength(size_t pos) const;
    uint16_t getRecordPositionIncrement(size_t pos) const;
    uint8_t* getRecordValuePtr(size_t pos) const;
// for testing
    const uint8_t* flashPos(size_t pos) { return _flashMemory + pos; } //!< for testing
    uint8_t flashPeek(size_t pos) { return _flashMemory[pos]; } //!< for testing
protected:
    uint8_t* _flashMemory {};
    size_t _flashMemorySize;
    uint32_t _sectorCount;
    std::array<uint8_t, PAGE_SIZE> _pageCache {};
};
