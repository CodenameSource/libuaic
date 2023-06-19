#pragma once

#include <stddef.h>
#include <assert.h>

typedef enum
{
    UAI_OK,
    UAI_ERRNO,
} UAI_Status;

#define UAI_MUST(status) assert(!(status));

