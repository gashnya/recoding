#include <stdlib.h>
#include <stdint.h>
#include "utf.h"
#include "memory.h"

encoding get_encoding(sized_char_array data) {
    if (data.size >= 4 && data.arr[0] == 0xFF && data.arr[1] == 0xFE && data.arr[2] == 0 && data.arr[3] == 0) {
        if (data.size % 4 == 0) {
            return U32L;
        }
        return U16L;
    }
    if (data.size >= 4 && data.arr[0] == 0 && data.arr[1] == 0 && data.arr[2] == 0xFE && data.arr[3] == 0xFF) {
        return U32B;
    }
    if (data.size >= 2 && data.arr[0] == 0xFF && data.arr[1] == 0xFE) {
        return U16L;
    }
    if (data.size >= 2 && data.arr[0] == 0xFE && data.arr[1] == 0xFF) {
        return U16B;
    }
    if (data.size >= 3 && data.arr[0] == 0xEF && data.arr[1] == 0xBB && data.arr[2] == 0xBF) {
        return U8;
    }
    return U8_NO_BOM;
}

sized_int_array u8_to_unicode(sized_char_array data) {
    sized_int_array res;
    res.size = data.size;
    res.arr = safe_malloc(sizeof(uint32_t) * res.size);

    const uint8_t start_mask[4] = {0b0u, 0b110u, 0b1110u, 0b11110u};
    const uint8_t start_val[4] = {0b11111111u, 0b00011111u, 0b00001111u, 0b00000111u};
    const uint8_t mid_mask = 0b10u;
    const uint8_t mid_val = 0b00111111u;

    size_t real_size = 0;
    for (size_t i = 0; i < data.size; i++) {
        uint32_t code = 0;
        int correct_start = 0;
        for (int s = 3; s >= 0; s--) {
            if ((data.arr[i] >> (6 - s + 1 * (!s))) == start_mask[s]
                    && data.arr[i] <= 0xF4 && data.arr[i] != 0xC0 && data.arr[i] != 0xC1) {
                code += (start_val[s] & data.arr[i]) << (6u * s);
                for (size_t m = 0; m < s; m++) {
                    i++;
                    if ((data.arr[i] >> 6) == mid_mask) {
                        code += (mid_val & data.arr[i]) << (6u * (s - m - 1));
                    } else {
                        code = 0xDC00 + data.arr[i];
                    }
                }
                correct_start = 1;
                break;
            }
        }
        if (!correct_start) {
            code = 0xDC00 + data.arr[i];
        }
        res.arr[real_size++] = code;
    }
    res.size = real_size;

    return res;
}

sized_int_array u16_to_unicode(sized_char_array data, endian endian) {
    sized_int_array res;
    res.size = data.size;
    res.arr = safe_malloc(sizeof(uint32_t) * res.size);
    size_t real_size = 0;

    uint8_t flag = (endian == BIG);
    for (size_t i = 0; i < data.size; i++) {
        uint32_t first;
        first = data.arr[i + 1 * (!flag)];
        first = (first << 8u) + data.arr[i + 1 * (flag)];
        if (!((first <= 0xD7FF) || (first >= 0xE000 && first <= 0xFFFF))) {
            if (first >= 0xDC00 && first <= 0xDFFF) {
                res.arr[real_size] = first;
                real_size++;
                i++;
                continue;
            }
            uint32_t second;
            second = data.arr[i + 2 + 1 * (!flag)];
            second = (second << 8u) + data.arr[i + 2 + 1 * (flag)];
            if ((second <= 0xD7FF) || (second >= 0xE000 && second <= 0xFFFF) ||
                    (second >= 0xD800 && second <= 0xDBFF)) {
                    res.arr[real_size] = first;
                    real_size++;
                    res.arr[real_size] = second;
                    i += 2;
                    real_size++;
                    continue;
            }
            first = ((first - 0xD800u) << 10u) + 0x10000;
            second = second - 0xDC00;
            res.arr[real_size] = first + second;
            i += 2;
        } else {
            res.arr[real_size] = first;
        }
        i++;
        real_size++;
    }
    res.size = real_size;
    return res;
}

sized_int_array u32_to_unicode(sized_char_array data, endian endian) {
    sized_int_array res;
    res.size = data.size / 4;
    res.arr = safe_malloc(sizeof(uint32_t) * res.size);

    for (size_t i = 0; i < data.size; i += 4) {
        if (endian == BIG) {
            res.arr[i / 4] = (data.arr[i] << 24u) + (data.arr[i + 1] << 16u) + (data.arr[i + 2] << 8u) + data.arr[i + 3];
        } else {
            res.arr[i / 4] = (data.arr[i + 3] << 24u) + (data.arr[i + 2] << 16u) + (data.arr[i + 1] << 8u) + data.arr[i];
        }
    }
    return res;
}

sized_char_array unicode_to_u8(sized_int_array data, int BOM) {
    sized_char_array res;
    res.size = data.size * 4 + 3 * BOM;
    res.arr = safe_malloc(sizeof(uint8_t) * res.size);

    if (BOM) {
        res.arr[0] = 0xEF;
        res.arr[1] = 0xBB;
        res.arr[2] = 0xBF;
    }
    size_t real_size = 3 * BOM;

    const uint32_t bounds_l[4] = {0, 0x80, 0x800, 0x10000};
    const uint32_t bounds_r[4] = {0x7F, 0x7FF, 0xFFFF, 0x10FFFF};
    const uint8_t start_prefix[4] = {0, 0b11000000, 0b11100000, 0b11110000};
    const uint8_t mid_prefix = 0b10000000;

    for (size_t i = 0; i < data.size; i++) {
        if (data.arr[i] >= 0xDC80 && data.arr[i] <= 0xDCff) {
            res.arr[real_size] = data.arr[i] - 0xDC00;
            real_size++;
            continue;
        }
        int nb = 0;
        for (int b = 0; b < 4; b++) {
            if (bounds_l[b] <= data.arr[i] && data.arr[i] <= bounds_r[b]) {
                nb = b + 1;
                break;
            }
        }
        for (int k = 0; k < nb - 1; k++) {
            res.arr[real_size + nb - 1 - k] = mid_prefix + (((0b111111u << (6u * k)) & data.arr[i]) >> (6u * k));
        }
        res.arr[real_size] = start_prefix[nb - 1] + (data.arr[i] >> (6u * (nb - 1)));
        real_size += nb;
    }

    res.size = real_size;
    return res;
}

sized_char_array unicode_to_u16(sized_int_array data, endian endian) {
    sized_char_array res;
    res.size = data.size * 4 + 2;
    res.arr = safe_malloc(sizeof(uint8_t) * res.size);
    size_t real_size = 2;
    uint8_t flag = (endian == BIG);
    if (flag) {
        res.arr[0] = 0xFE;
        res.arr[1] = 0xFF;
    } else {
        res.arr[0] = 0xFF;
        res.arr[1] = 0xFE;
    }
    for (size_t i = 0; i < data.size; i++) {
        if ((data.arr[i] <= 0xD7FF) || (data.arr[i] >= 0xE000 && data.arr[i] <= 0xFFFF) ||
                (data.arr[i] >= 0xD800 && data.arr[i] <= 0xDFFF)) {
            res.arr[real_size + 1 * (!flag)] = (data.arr[i] >> 8u);
            res.arr[real_size + 1 * (flag)] = ((data.arr[i] << 8u) >> 8u);
            real_size += 2;
        } else if (data.arr[i] > 0xFFFF) {
            uint16_t first = 0xD800u + ((data.arr[i] - 0x10000u) >> 10u);
            uint16_t second = 0xDC00u + (((data.arr[i] - 0x10000u) << 22u) >> 22u);

            res.arr[real_size + 1 * (!flag)] = (first >> 8u);
            res.arr[real_size + 1 * (flag)] = ((first << 8u) >> 8u);
            res.arr[real_size + 2 + 1 * (!flag)] = (second >> 8u);
            res.arr[real_size + 2 + 1 * (flag)] = ((second << 8u) >> 8u);

            real_size += 4;
        }
    }
    res.size = real_size;
    return res;
}

sized_char_array unicode_to_u32(sized_int_array data, endian endian) {
    sized_char_array res;
    res.size = data.size * 4 + 4;
    res.arr = safe_malloc(sizeof(uint8_t) * res.size);

    uint8_t flag = (endian == BIG);
    if (flag) {
        res.arr[0] = 0;
        res.arr[1] = 0;
        res.arr[2] = 0xFE;
        res.arr[3] = 0xFF;
    } else {
        res.arr[0] = 0xFF;
        res.arr[1] = 0xFE;
        res.arr[2] = 0;
        res.arr[3] = 0;
    }

    for (size_t i = 4; i < (data.size + 1) * 4; i += 4) {
        if (endian == BIG) {
            res.arr[i] = (data.arr[(i - 1) / 4] >> 24u);
            res.arr[i + 1] = ((data.arr[(i - 1) / 4] << 8u) >> 24u);
            res.arr[i + 2] = ((data.arr[(i - 1) / 4] << 16u) >> 24u);
            res.arr[i + 3] = ((data.arr[(i - 1) / 4] << 24u) >> 24u);
        } else {
            res.arr[i + 3] = (data.arr[(i - 1) / 4] >> 24u);
            res.arr[i + 2] = ((data.arr[(i - 1) / 4] << 8u) >> 24u);
            res.arr[i + 1] = ((data.arr[(i - 1) / 4] << 16u) >> 24u);
            res.arr[i] = ((data.arr[(i - 1) / 4] << 24u) >> 24u);
        }
    }
    return res;
}

sized_char_array change_encoding(sized_char_array old, encoding old_encoding, encoding new_encoding) {
    const uint8_t bom_length[6] = {0, 3, 2, 2, 4, 4};
    old.arr += bom_length[old_encoding];
    old.size -= bom_length[old_encoding];

    sized_int_array unicode;
    switch (old_encoding) {
        case U8_NO_BOM:
        case U8:
            unicode = u8_to_unicode(old);
            break;
        case U16L:
            unicode = u16_to_unicode(old, LITTLE);
            break;
        case U16B:
            unicode = u16_to_unicode(old, BIG);
            break;
        case U32L:
            unicode = u32_to_unicode(old, LITTLE);
            break;
        case U32B:
            unicode = u32_to_unicode(old, BIG);
            break;
        default:
            break;
    }

    sized_char_array res;
    switch (new_encoding) {
        case U8_NO_BOM:
            res = unicode_to_u8(unicode, 0);
            break;
        case U8:
            res = unicode_to_u8(unicode, 1);
            break;
        case U16L:
            res = unicode_to_u16(unicode, LITTLE);
            break;
        case U16B:
            res = unicode_to_u16(unicode, BIG);
            break;
        case U32L:
            res = unicode_to_u32(unicode, LITTLE);
            break;
        case U32B:
            res = unicode_to_u32(unicode, BIG);
            break;
        default:
            break;
    }
    return res;
}
