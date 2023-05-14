#include <stdio.h>
#include <stdint.h>
#include "memory.h"
#include "utils.h"
#include "utf.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: recode <input_file> <output_file> <output_encoding>\n");
        return ERROR;
    }

    const char *in_file = argv[1];
    const char *out_file = argv[2];
    FILE *in = safe_open(in_file, "rb");

    sized_char_array old;
    old.size = get_size(in);
    old.arr = safe_malloc(old.size * sizeof(uint8_t));

    fread(old.arr, sizeof(char), old.size, in);

    safe_close(in, in_file);

    encoding old_encoding = get_encoding(old);
    encoding new_encoding = (int)argv[3][0] - 48;

    sized_char_array new = change_encoding(old, old_encoding, new_encoding);

    FILE *out = safe_open(argv[2], "wb");

    fwrite(new.arr, sizeof(char), new.size, out);

    safe_close(out, out_file);

    free_all();

    return 0;
}
