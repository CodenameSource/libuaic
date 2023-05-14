#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/uai/data.h"

UAI_Status df_load_csv(DataFrame *df, const char *filename, char sep)
{
    int fd = open(filename, O_RDONLY);
    if (!fd)
        return UAI_ERRNO;

    UAI_Status ret;

    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0)
    {
        ret = UAI_ERRNO;
        goto end;
    }

    char *strbuf = malloc(size + 1);
    if (!strbuf)
    {
        ret = UAI_ERRNO;
        goto end;
    }

    if (lseek(fd, 0, SEEK_SET) < 0)
    {
        free(strbuf);
        ret = UAI_ERRNO;
        goto end;
    }
    if (read(fd, strbuf, size) != size)
    {
        free(strbuf);
        ret = UAI_ERRNO;
        goto end;
    }

    df->strbuf = strbuf;
    df->strbuf_size = size;

    ret = UAI_OK;
end:
    close(fd);
    return ret;
}

