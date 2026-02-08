# Flash Key Length Value (KLV) library ![license](https://img.shields.io/badge/license-MIT-green) ![open source](https://badgen.net/badge/open/source/blue?icon=github)

The FlashKlv library allows you to store data permanently on Flash memory.
The library stores values as Key Length Value records.
It can be used to save data like API keys, configuration data, network credentials, or any other data that you want to save permanently.

## Usage

Data is stored in Key Length Value format, using either 8-bit keys or 16-bit keys, as follows:

| Key                        | Storage | Max length of value |
| -------------------------- | ------- | ------------------- |
| 0x01-0x3F (1-63)           | 8-bit   | 255 bytes           |
| 0x0100-0x1FFF (256-8191)   | 16-bit  | 255 bytes           |
| 0x2000-0x3FFD (8192-16361) | 16-bit  | 65535 bytes         |

Other keys are invalid and may not be used. In particular keys of 0, 64-255, and > 16361 are invalid.

This gives a total of 16,169 usable keys.

Keys 0x01-0x3F (1-63): the key and the length are stored as 8-bit values and so there is an
overhead of 2 bytes per record stored.

Keys 0x0100-0x1FFF (256-8191): the key is stored as a 16-bit value and the length is stored as a 8-bit value and so there is an
overhead of 3 bytes per record stored.

Keys 0x2000-0x3FFD (8192-16361): the key and the length are stored as 16-bit values and so there is an
overhead of 4 bytes per record stored.

### How much memory should I allocate for FlashTLV?

FlashTLV will work using just a single sector (where a sector is the minimum erasable size of flash, 4096 bytes on the Raspberry Pi Pico)
when using one bank of flash, or two sectors if using two banks of flash.

However flash memory wears out if continually programmed and erased, and the more memory allocated for FlashTLV the less localized this wear is.

So I recommend you allocate all the memory that is not otherwise used to FlashTLV. There is no disadvantage to doing this (since the
memory is not otherwise use) and this will mean that the flash fills up less frequently and so wear is reduced. Indeed for some applications
it may be the case the flash never fills up.

So if you have 256KB of flash left over after you have programmed your Pico, then I recommend you allocate all of this 256KB
(that is 256/4 = 64 sectors) to FlashTLV.

eg:

```cpp
enum { SECTOR_COUNT = 64 };
FlashKlv flashKLV(SECTOR_COUNT);
```

## Example

This example shows creating a FlashKlv object, writing a record, and then reading back that record.

```cpp
// create a FlashKlv object
enum { SECTOR_COUNT = 64 };
FlashKlv flashKLV(SECTOR_COUNT);

// declare a key and structure
enum { CONFIG_KEY = 0x01 };
struct config_t {
    uint16_t a;
    uint8_t b;
    uint8_t c;
};

// write s config structure to flash
const config_t configW = { .a = 713, .b =27, .c = 12 };
int32_t err = flashKLV.write(CONFIG_KEY, sizeof(configW), &configW);
TEST_ASSERT_EQUAL(FlashKlv::OK, err);

// read a config structure
config_t configR {};
err = flashKLV.read(&configR, sizeof(configR), CONFIG_KEY);
TEST_ASSERT_EQUAL(FlashKlv::OK, err);

// test the values are as expected
TEST_ASSERT_EQUAL(713, configR.a);
TEST_ASSERT_EQUAL(27, configR.b);
TEST_ASSERT_EQUAL(12, configR.c);
```

## What happens when the flash memory fills up

When the flash memory fills up it typically contains deleted records. These deleted records take up space which can be recovered.

One way to recover this space is to erase the flash, and then rewrite all the records from values held in memory.

An alternative approach is to divide the flash into two banks. When one bank fills up, erase the other bank,
copy the records from the used bank to the other bank, and then swap banks. An example of this approach is given
below:

```cpp
// create a FlashKlv object with two banks of flash
enum { SECTOR_COUNT = 32 };
FlashKlv flashKLV(SECTOR_COUNT, FlashKlv::TWO_BANKS);

...

const config_t configW = { .a = 713, .b =27, .c = 12 };
int32_t err = flashKLV.write(CONFIG_KEY, sizeof(configW), &configW);
if (err == ERROR_FLASH_FULL) {
    // the current bank is full, so erase the other bank and copy the records to that bank
    flashKLV.erase_other_bank();
    flashKLV.copy_records_to_other_bank_and_swap_banks();
    // we are now using the other bank, so we should have recovered enough flash to write the  record
    err = flashKLV.write(CONFIG_KEY, sizeof(configW), &configW);
    TEST_ASSERT_GREATER_OR_EQUAL(FlashKlv::OK, err);
}
// carry on using the new bank
...
```

## Implementation of FlashTLV

The implementation is designed to be "flash friendly", that is minimize the requirement to erase flash before writing new values.

**To do this it uses the fact that a flash bit may be changed from `1` to `0` without needing to erase the flash.**

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

Keys can be either 16-bit or 8-bit and are stored in bigendian format (Most Significant Byte (MSB) first), since for 8-bit keys
there is no Least Significant Byte (LSB).

Two flags are used to control the interpretation of a key:

1. If bit 7 is set then the key represents a record. If bit 7 is clear, then it means the record has been deleted.
2. If bit 6 is set then the key is a 16-bit key, if bit 6 is clear then the key is a 8-bit key.

| Key    | Interpretation                               |
| ------ | -------------------------------------------- |
| 0x01   | 8-bit key, value 0x01, record deleted        |
| 0x81   | 8-bit key, value 0x01, record not deleted    |
| 0x0317 | 16-bit key, value 0x0317, record deleted     |
| 0x8317 | 16-bit key, value 0x0317, record not deleted |

Additionally the keys `0x00`, `0xFFFE`, and `0xFFFF` are used internally.

This restricts 8-bit keys to the range [`0x01`,`0x3F`], and 16-bit keys to the range [`0x0100`,`0x3FFD`].

### Example using 8-bit key

To give an example, consider a 4-byte record with the key `0x01` and value `{ 0x1A, 0x2B, 0x3C, 0x4D }`.
This key value means that the key and the length will be stored as 8-bit values.

So the first 12 bytes of flash will be:

```cpp
{ 0x81, 0x04, 0x1A, 0x2B, 0x3C, 0x4D } // 0x81 is key(0x01) with bit 7 set, 0x04 is length
{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } // unused flash
```

Suppose we now want to store a record with key `0x01` and value `{ 0x3A, 0x72, 0xFF, 0x11 }`.
We cannot overwrite the old record with this value, so we must mark the old record as deleted,
by clearing its top bit.
After doing this and writing the new record the first 12 bytes of flash will be:

```cpp
{ 0x01, 0x04, 0x1A, 0x2B, 0x3C, 0x4D } // 0x01 is key with bit 7 cleared, ie marked as deleted, 0x04 is length
{ 0x81, 0x04, 0x3A, 0x72, 0xFF, 0x11 } // 0x81 is key with bit 7 set, 0x04 is length
```

To find a record, we walk the flash (using the length values to move from record to record), until we find the record
with our desired key and the top bit set (indicating it is not a deleted record).

### Example using 16-bit key

If we choose a key greater than 255 then key and length will be stored as 16-bit values.
So to repeat the example above using a 16-bit key, let's choose the key `0x0721`.
If we write to an empty flash, then the first 16 bytes of flash will be:

```cpp
{ 0xC7, 0x21, 0x04, 0x00, 0x1A, 0x2B, 0x3C, 0x4D } // 0xC7 is MSB of key(0x07) with bits 6 and 7 set, 0x21 is LSB of key , 0x04, 0x00 is length
{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } // unused flash
```

Note that the key is stored in bigendian format, and that bit 7 is set to denote undeleted and bit 6 is set to denote 16-bit key.
This is the reason 8-bit keys are must be less than 64: only bits 0 to 6 are available for the key.

Suppose we now want to store a record with key `0x0721` and value `{ 0x3A, 0x72, 0xFF, 0x11 }`.
We cannot overwrite the old record with this value, so we must mark the old record as deleted, by clearing its top bit.
After doing this and writing the new record the first 16 bytes of flash will be:

```cpp
{ 0x47, 0x21, 0x04, 0x00, 0x1A, 0x2B, 0x3C, 0x4D } // 0x47 is MSB of key(0x07) with bit 6 set and bit 7 cleared, 0x21 is LSB of key, 0x04, 0x00 is length
{ 0xC7, 0x21, 0x04, 0x00, 0x1A, 0x2B, 0x3C, 0x4D } // 0xC7 is MSB of key(0x07) with bits 6 and 7 set, 0x21 is LSB of key, 0x04, 0x00 is length
```

## Further reading

[Read and write data with the Pi Pico onboard flash](https://www.makermatrix.com/blog/read-and-write-data-with-the-pi-pico-onboard-flash/)

[EEPROM Library](https://arduino-pico.readthedocs.io/en/latest/eeprom.html)

[A little fail-safe filesystem designed for microcontrollers](https://github.com/littlefs-project/littlefs)
