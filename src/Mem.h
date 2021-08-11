#ifndef MEM_H
#define MEM_H

#if defined(__linux__)
// For `size_t`
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// These functions call `malloc` or `realloc` accordingly.
// If error, print to `stderr` and exit
void *Mem_Alloc(size_t size);
void *Mem_Realloc(void *ptr, size_t newSize);

#ifdef __cplusplus
}
#endif

#endif
