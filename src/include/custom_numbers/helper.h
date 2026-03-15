#pragma once

#include <stddef.h>
#include <sys/cdefs.h>

__always_inline void *move(void **self)
{
    void *tmp = *self;
    *self = NULL;
    return tmp;
}
