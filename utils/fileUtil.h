#pragma once

#include <cstdio>
#include <cstdint>

#include "panic.h"

size_t getFileSize(FILE *file)
{
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    return size;
}
// function reads the file copies its contents inputArray a buffer and returns the pointer to the buffer
static uint8_t *readFile(char const *book1, size_t *out_size)
{
    FILE *f = fopen(book1, "rb");
    if (!f)
        panic("file not found: %s\n", book1);

    size_t size = getFileSize(f); // get file size

    uint8_t *buf = new uint8_t[size]; // allocates memory for the block dynamically
    if (fread(buf, size, 1, f) != 1)
        panic("read failed\n");

    fclose(f);
    if (out_size)
        *out_size = size;

    return buf;
}