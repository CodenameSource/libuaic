#pragma once

#include <assert.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

// TODO: learn what inline does hihi
void setup_cwd(const char *file_macro)
{
    // TOOD: check if dirname() usage is correct
    char *cwd = strdup(file_macro), *dname = dirname(cwd);
    assert(0 <= chdir(dname));
    free(cwd);
}
