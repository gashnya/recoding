#pragma once
#include "utils.h"

typedef enum {
    U8_NO_BOM = 0,
    U8,
    U16L,
    U16B,
    U32L,
    U32B
} encoding;

typedef enum {
    LITTLE,
    BIG
} endian;

encoding get_encoding(sized_char_array data);

sized_int_array u8_to_unicode(sized_char_array data);

sized_int_array u16_to_unicode(sized_char_array data, endian endian);

sized_int_array u32_to_unicode(sized_char_array data, endian endian);

sized_char_array unicode_to_u8(sized_int_array data, int BOM);

sized_char_array unicode_to_u16(sized_int_array data, endian endian);

sized_char_array unicode_to_u32(sized_int_array data, endian endian);

sized_char_array change_encoding(sized_char_array old, encoding old_encoding, encoding new_encoding);
