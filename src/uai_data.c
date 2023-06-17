#include <assert.h>
#include <stdbool.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>

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

static inline uint16_t to_big_endian(uint16_t val)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (((val & 0x00FF) << CHAR_BIT) |
            ((val & 0xFF00) >> CHAR_BIT));
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return val;
#else
#    error unsupported endianness
#endif
}

static bool is_crlf(const char *p)
{
    static_assert(CHAR_BIT == 8, "char is not 8 bits long, but we kinda expect it to be ;-;");
    return to_big_endian(*(uint16_t *)p) == ('\r' << CHAR_BIT | '\n');
}

static bool is_eol(const char *p)
{
    return *p == '\n' || is_crlf(p);
}

struct lexer
{
    char *p, *end, sep;
};

static void skip_field(struct lexer *l)
{
    while (l->p < l->end && *l->p != l->sep && !is_eol(l->p))
        ++l->p;
}

static void skip_and_escape_field(struct lexer *l)
{
    for (char *q=l->p, *prev=l->p;
            l->p < l->end && (*l->p != '"' || l->p[1] == '"' || *prev == '"');
            prev=l->p, ++l->p)
    {
        *q = *l->p;
        if (*l->p != '"' || l->p[1] == '"')
            ++q;
    }
    if (l->p < l->end)
        *l->p++ = 0;
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

    DataCell *cellbuf = malloc(sizeof *cellbuf * num_rows * num_cols);
    if (!cellbuf)
    {
        err = UAI_ERRNO;
        goto error_post_strbuf;
    }
    DataCell **rows = malloc(sizeof *rows * num_rows);
    if (!rows)
    {
        err = UAI_ERRNO;
        goto error_post_cellbuf;
    }

    struct lexer l = { strbuf, strbuf+size, sep };
    for (size_t row = 0; row < num_rows; ++row)
    {
        rows[row] = cellbuf + row * num_cols;
        for (size_t col = 0; col < num_cols; ++col)
        {
            if (*l.p == '"')
            {
                rows[row][col].type = DATACELL_STR;
                rows[row][col].as_str = ++l.p;
                skip_and_escape_field(&l);
            }
            else
            {
                rows[row][col].type = DATACELL_STR;
                rows[row][col].as_str = l.p;
                skip_field(&l);
            }
            assert(l.p < l.end  &&  *l.p == l.sep || is_eol(l.p));
            if (*l.p == l.sep)
                *l.p++ = 0;
        }
        assert(is_eol(l.p));
        char c = *l.p;
        *l.p = 0;
        l.p += c == '\n' ? 1 : 2;
    }

    df->strbuf = strbuf;
    df->strbuf_size = size;

    df->rows = num_rows, df->cols = num_cols;
    df->cellbuf = cellbuf;
    df->data = rows;

    close(fd);
    return UAI_OK;

//error_post_rows:
    //free(rows);
error_post_cellbuf:
    free(cellbuf);
error_post_strbuf:
    free(strbuf);
error_post_fd:
    close(fd);
    return err;
}

void df_set_header(DataFrame *df, bool value)
{
    // HACK: incrementing the data, may pass invalid block to free()!!!
    if (value && !df->header)
        df->header=*df->data, ++df->data, --df->rows;
    else if (!value && df->header)
        df->header=NULL, --df->data, ++df->rows;
}

UAI_Status df_copy(const DataFrame *original, DataFrame *copy)
{
    size_t num_rows = original->rows + !!original->header, num_cols=original->cols;
    char *strbuf = malloc(original->strbuf_size + 1);
    if (!strbuf)
        return UAI_ERRNO;
    memcpy(strbuf, original->strbuf, original->strbuf_size + 1);

    UAI_Status err = UAI_OK;

    DataCell *cellbuf = malloc(sizeof *cellbuf * num_rows*num_cols);
    if (!cellbuf)
    {
        err = UAI_ERRNO;
        goto error_post_strbuf;
    }
    memcpy(cellbuf, original->cellbuf, sizeof *cellbuf * num_rows*num_cols);

    DataCell **rows = malloc(sizeof *rows * num_rows);
    if (!rows)
    {
        err = UAI_ERRNO;
        goto error_post_cellbuf;
    }
    for (size_t row=0; row<num_rows; ++row)
        rows[row] = cellbuf + row * num_cols;

    copy->rows = original->rows, copy->cols = original->cols;
    copy->cellbuf = cellbuf;
    copy->strbuf_size = original->strbuf_size;
    copy->strbuf = strbuf;
    if (original->header)
        copy->data = rows+1, copy->header = *rows;
    else
        copy->data = rows,   copy->header = NULL;

    return UAI_OK;

error_post_cellbuf:
    free(cellbuf);
error_post_strbuf:
    free(strbuf);
    return err;
}

UAI_Status df_create(DataFrame *df, size_t num_rows, size_t num_cols)
{
    DataCell *cellbuf = malloc(sizeof *cellbuf * num_rows * num_cols);
    if (!cellbuf)
        return UAI_ERRNO;

    UAI_Status err = UAI_OK;

    DataCell **rows = malloc(sizeof *rows * num_rows);
    if (!rows)
    {
        err = UAI_ERRNO;
        goto error_post_cellbuf;
    }

    for (size_t r=0; r<num_rows; ++r)
    {
        rows[r] = cellbuf + r * num_cols;
        for (size_t c=0; c<num_cols; ++c)
            rows[r][c].type = DATACELL_NAN;
    }

    df->rows = num_rows, df->cols = num_cols;
    df->strbuf = NULL, df->strbuf_size = 0;
    df->header = NULL;
    return UAI_OK;

error_post_cellbuf:
    free(cellbuf);
    return err;
}

const char *skip_spaces(const char *s)
{
    while (isspace(*s))
        s++;
    return s;
}

void df_range_to_double(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col, enum DataCell_ConvertStrictness strictness)
{
    assert(start_row <= end_row && end_row < df->rows);
    assert(start_col <= end_col && end_col < df->cols);
    for (size_t r=start_row; r <= end_row; ++r)
        for(size_t c=start_col; c <= end_col; ++c)
        {
            if (df->data[r][c].type != DATACELL_STR)
                continue;
            char *rest;
            double val = strtod(df->data[r][c].as_str, &rest);
            // TODO: error checking (HUGE_VAL, ERANGE errno)
            // FIXME: strict conversions are strict about trailing whitespace: "1   ",
            //        but not leading whitespace
            if ((strictness == DATACELL_CONVERT_LAX && rest != df->data[r][c].as_str) ||
                    (rest && !*skip_spaces(rest)))
            {
                df->data[r][c].type = DATACELL_DOUBLE;
                df->data[r][c].as_double = val;
            }
            else
                df->data[r][c].type = DATACELL_NAN;
        }
}

void df_col_to_double(DataFrame *df, size_t col, enum DataCell_ConvertStrictness strictness)
{
    df_range_to_double(df, 0,col, df->rows-1,col, strictness);
}

void df_all_to_double(DataFrame *df, enum DataCell_ConvertStrictness strictness)
{
    df_range_to_double(df, 0, 0, df->rows-1, df->cols-1, strictness);
}

void df_destroy(DataFrame *df)
{
    free(df->strbuf);
    free(df->cellbuf);
    // HACK: because of the implementation of the header, we need to pass a
    // decremented pointer to free()
    free(df->header ? df->data-1 : df->data);
    *df = (DataFrame){ 0 };
}
