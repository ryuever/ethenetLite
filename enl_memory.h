#ifndef ENL_MEMORY_H
#define ENL_MEMORY_H

#include "enl_common.h"

void* enl_malloc(int size);
void enl_free(void* memory);

#endif  /* ENL_MEMORY_H */