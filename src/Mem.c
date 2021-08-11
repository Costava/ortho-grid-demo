#include "Mem.h"

#include <stdio.h>
#include <stdlib.h>

void *Mem_Alloc(size_t size) {
    void *const ptr = malloc(size);

    if (size != 0 && ptr == NULL) {
        fprintf(stderr, "%s: Failed to malloc %zu bytes\n", __func__, size);
        exit(1);
    }

    return ptr;
}

void *Mem_Realloc(void *ptr, size_t newSize) {
    ptr = realloc(ptr, newSize);

    if (newSize != 0 && ptr == NULL) {
        fprintf(stderr, "%s: Failed to realloc to %zu bytes\n", __func__, newSize);
        exit(1);
    }

    return ptr;
}
