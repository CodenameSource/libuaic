#include <assert.h>
#include <stdbool.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/uai/data.h"

enum { NO_CHAR = -2 };

static UAI_Status read_char(int fd, int *prev_char, int *current_char, int *next_char)
{
    do
    {
        char c;
        ssize_t nread = *next_char != EOF  ?  read(fd, &c, 1)  :  0;
        if (nread < 0)
            return UAI_ERRNO;
        *prev_char = *current_char, *current_char = *next_char;
        *next_char = nread ? c : EOF;
    } while (*current_char == NO_CHAR);
    return UAI_OK;
}

UAI_Status df_load_csv(DataFrame *df, const char *filename, char sep)
{
    assert(sep != '"' && "Cannot use \" as separator");

    int fd = open(filename, O_RDONLY);
    if (!fd)
        return UAI_ERRNO;

    UAI_Status err;

    size_t num_rows=0, num_cols=0;
    {
        bool in_quote = false;
        for (int prev_c=NO_CHAR, c=NO_CHAR, next_c=NO_CHAR; err=read_char(fd, &prev_c, &c, &next_c), c!=EOF;)
        {
            if (err)
                goto error_post_fd;
            if (!in_quote)
            {
                if (c == '"')
                    assert(prev_c==NO_CHAR || prev_c==sep), in_quote=true;
                else if (c == sep)
                    num_cols += !num_rows;
                else if (c == '\n' || next_c == EOF)
                    num_cols += !num_rows, num_rows++;
            }
            else
            {
                if (prev_c != '"' && c == '"' && next_c != '"')
                    assert(next_c==EOF || next_c==sep), in_quote = false;
                else
                    assert(next_c!=EOF);
            }
        }
    }

    off_t size = lseek(fd, 0, SEEK_CUR);
    if (size < 0)
    {
        err = UAI_ERRNO;
        goto error_post_fd;
    }
    if (lseek(fd, 0, SEEK_SET) < 0)
    {
        err = UAI_ERRNO;
        goto error_post_fd;
    }

    // padding with an extra 0, so we can safely process the file 2 chars at a time
    // (i.e. when parsing "\r\n"), even if its size is uneven
    char *strbuf = malloc(size + 1);
    if (!strbuf)
    {
        err = UAI_ERRNO;
        goto error_post_fd;
    }

    if (read(fd, strbuf, size) != size)
    {
        err = UAI_ERRNO;
        goto error_post_strbuf;
    }
    strbuf[size] = 0;

    df->strbuf = strbuf;
    df->strbuf_size = size;

    df->rows = num_rows, df->cols = num_cols;

    close(fd);
    return UAI_OK;

error_post_strbuf:
    free(strbuf);
error_post_fd:
    close(fd);
    return err;
}

