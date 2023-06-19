#pragma once

#include <assert.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

// TODO: learn what inline does hihi
void setup_cwd()
{
    char *cwd = strdup(__FILE__), *dname = dirname(cwd);
    assert(0 <= chdir(dname));
    free(cwd);
}

#define UAI_MUST(status) assert(!(status));

