### Recoding

This program recodes provided text file.

* Command-line arguments: input file, output file and output encoding, where output encoding is:

  | Command-line argument |   Output encoding    |
  | :-------------------: | :------------------: |
  |           0           |  UTF-8 without BOM   |
  |           1           |    UTF-8 with BOM    |
  |           2           | UTF-16 Little Endian |
  |           3           |  UTF-16 Big Endian   |
  |           4           | UTF-32 Little Endian |
  |           5           |  UTF-32 Big Endian   |

* Improper UTF-8 bytes are encoded/decoded as symbols from range 0xDC80..0xDCFF. UTF-8 -> another encoding -> UTF-8 returns the source file (give or take BOM).

* This program does not use external libraries, correctly releases resources and handles errors.

Compilation:

```bash
gcc -o recode main.c memory.c utf.c utils.c
```

Execution:

```bash
./recode <input_file> <output_file> <output_encoding>
```

