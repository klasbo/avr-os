#pragma once

#include <stddef.h>

void* os_malloc(size_t len) __attribute__((malloc));
void os_free(void* ptr);
void os_realloc(void *ptr, size_t len);