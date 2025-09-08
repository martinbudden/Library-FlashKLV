# Flash Key Length Value (KLV) library ![license](https://img.shields.io/badge/license-MIT-green) ![open source](https://badgen.net/badge/open/source/blue?icon=github)

The FlashKLV library allows you to store data permanently on Flash memory.
The library stores values as KeyLengthValue records.
It can be used to save data like API keys, configuration data, network credentials, or any other data that you want to save permanently.

## Usage

Data is stored in Key Length Value format, using either 8-bit keys or 16-bit keys, as follows:

| Key                       | Storage | Max length of value |
| ------------------------- | ------- | ------------------- |
| 1-63 (0x01-0x3F)          | 8-bit   | 255 bytes           |
| 256-16363 (0x0100-0x3FFF) | 16-bit  | 65535 bytes         |

Other keys are invalid and may not be used. In particular keys of 0, 64-255, and > 16363 are invalid.

This gives a total of 16,171 usable keys.

For keys 1 to 63, the key and the length are stored as 8-bit values and so there is an overhead of 2 bytes per record stored.

For keys 256 to 16363, the key and the length are stored as 16-bit values and so there is an overhead of 4 bytes per record stored.

## Example

```cpp
    // create a FlashKLV object
    enum { SECTOR_COUNT = 2 };
    FlashKLV flashKLV(FlashKLV::SECTOR_SIZE*SECTOR_COUNT);

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
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    // read a config structure
    config_t configR {};
    err = flashKLV.read(&configR, sizeof(configR), CONFIG_KEY);
    TEST_ASSERT_EQUAL(FlashKLV::OK, err);

    // test the values are as expected
    TEST_ASSERT_EQUAL(713, configR.a);
    TEST_ASSERT_EQUAL(27, configR.b);
    TEST_ASSERT_EQUAL(12, configR.c);
```

## What happens when the flash memory fills up

When the flash memory fills up it must be erased before new data can be written.

The library provides `eraseSector` and `eraseAllSectors` functions to support this, but the specifics
of how to handle this is application dependent.

Two possible strategies are:

1. Erase all sectors, and rewrite the data from values held in memory
2. Have two banks of flash, when the first one fills up, read from the first one and write to the second.
   Then erase the first bank, ready for when the second bank fills up.

## Implementation

The implementation is designed to be "flash friendly", that is minimize the requirement to erase flash before writing new values.

To do this it uses the fact that a flash bit may be changed from `1` to `0` without requiring to erase the flash.

Initially the flash memory is set to all `0xFF`. On writing a record some of these bits are cleared.

Writing a record involves the following:

1. the implementation checks if a record of with the same key, length, and value exists: if is does then
   there is no need to re-write the record.
2. If a record exists with the same key and length, then it will be overwritten in place if this is possible.
   So for example the value `{ 0xFF, 0x3F, 0x00, 0x01 }` may be overwritten with `{ 0x23, 0x01, 0x00, 0x00 }`,
   since this only involves flipping bits from `1` to `0`.
3. If the old record cannot be overwritten, then it will be marked as deleted (by clearing the top bit of the key),
   and a new record will be written at the end of the file.

Reading a record involves walking the file, skipping over deleted records, until a record with the specified key is found.

To give an example, consider a 4-byte record with the key `0x01` and value `{ 0x1A, 0x2B, 0x3C, 0x4D }`. This key value means
that the key and the length will be stored as 8-bit values.

So the first 12 bytes of flash will be:

```cpp
{ 0x81, 0x04, 0x1A, 0x2B, 0x3C, 0x4D } // 0x81 is key(0x01) with top bit set, 0x04 is length
{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } // unused flash
```

Suppose we now want to store a record with key `0x01` and value `{ 0x3A, 0x72, 0xFF, 0x11 }`.
We cannot overwrite the old record with this value, so we must mark the old record as deleted, by clearing
its top bit. After doing this and writing the new record the first 12 bytes of flash will be:

```cpp
{ 0x01, 0x04, 0x1A, 0x2B, 0x3C, 0x4D } // 0x01 is key with top bit cleared, ie marked as deleted, 0x04 is length
{ 0x81, 0x04, 0x3A, 0x72, 0xFF, 0x11 } // 0x81 is key with top bit set, 0x04 is length
```

To find a record, we walk the flash (using the length values to move from record to record), until we find the record
with our desired key and the top bit set (indicating it is not a deleted record).

Bit 6 (`0100 0000`, `0x40`) of the key set if a 16-bit key and length is being used.
