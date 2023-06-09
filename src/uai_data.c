#include <assert.h>
#include <stdbool.h>
#include <malloc.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "../include/uai/data.h"
#include "../include/uai/scaling.h"

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
    char *q=l->p;
    for (char prev=0;
            l->p < l->end && (*l->p != '"' || l->p[1] == '"' || prev == '"');
            prev=*l->p, ++l->p)
    {
        *q = *l->p;
        if (*l->p != '"' || l->p[1] == '"')
            ++q;
    }
    if (l->p < l->end)
        *q = 0, ++l->p;
}

UAI_Status df_load_csv(DataFrame *df, const char *filename, char sep)
{
    assert(sep != '"' && "Cannot use \" as separator");
    assert(sep != '\n' && "Cannot use \\n as separator");

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

UAI_Status df_create_hsplit(DataFrame *src, DataFrame *dst, size_t take, enum DataFrame_Sampling sampling)
{
    assert(take <= src->rows);

    if (sampling == DATAFRAME_SAMPLE_RAND)
        df_shuffle_rows(src);

    UAI_Status err = df_copy(src, dst);
    if (err)
        return err;

    src->rows -= take;
    memmove(*dst->data, *dst->data + (dst->rows - take) * dst->cols, sizeof **dst->data * take * dst->cols);
    dst->rows = take;
    return UAI_OK;
}

UAI_Status df_create_vsplit(DataFrame *src, DataFrame *dst, size_t take, enum DataFrame_Sampling sampling)
{
    assert(take <= src->rows);
    assert(sampling == DATAFRAME_SAMPLE_SEQ && "Only sequential sampling is supported");

    size_t new_rows = src->rows + !!src->header;
    DataCell *new_cellbuf = malloc(sizeof *new_cellbuf * new_rows * take);
    if (!new_cellbuf)
        return UAI_ERRNO;

    UAI_Status err = df_copy(src, dst);
    if (err)
        goto error_post_new_cellbuf;

    src->cols -= take;
    for (DataCell *s = dst->cellbuf + dst->cols - take, *d = new_cellbuf; d < new_cellbuf + new_rows * take; d += take, s += dst->cols)
        memcpy(d, s, sizeof *d * take);
    dst->cols = take;
    free(dst->cellbuf);
    dst->cellbuf = new_cellbuf;
    DataCell **rows = dst->header ? dst->data-1 : dst->data;
    for (size_t r=0; r<new_rows; ++r)
        rows[r] = new_cellbuf + r * take;
    return UAI_OK;

error_post_new_cellbuf:
    free(new_cellbuf);
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

UAI_Status df_export_csv(DataFrame *df, const char *filename, char sep)
{
    assert(sep != '"' && "Cannot use \" as separator");
    assert(sep != '\n' && "Cannot use \\n as separator");

    FILE *f = fopen(filename, "w");
    if (!f)
        return UAI_ERRNO;

    // HACK: truncatng a size_t, fails to work for numers > SSIZE_MAX
    for (ssize_t r = -!!df->header; r < (ssize_t)df->rows; ++r)
    {
        for (size_t c=0; c<df->cols; ++c)
        {
            switch (df->data[r][c].type)
            {
                case DATACELL_DOUBLE:
                    fprintf(f, "%f", df->data[r][c].as_double);
                    break;
                case DATACELL_STR:
                    if (strchr(df->data[r][c].as_str, sep) || strchr(df->data[r][c].as_str, '\n'))
                    {
                        fputc('"', f);
                        for (const char *p = df->data[r][c].as_str; *p; ++p)
                        {
                            if (*p == '"')
                                fputs("\"\"", f);
                            else
                                fputc(*p, f);
                        }
                        fputc('"', f);
                    }
                    else
                        fputs(df->data[r][c].as_str, f);
                    break;
                case DATACELL_NAN:
                    break;
            }
            if (c != df->cols-1)
                fputc(sep, f);
        }
        fputc('\n', f);
    }

    if (fclose(f) < 0)
        return UAI_ERRNO;
    return UAI_OK;
}

static DataCell *find_next_cell(DataFrame *df, size_t col, size_t start_row, size_t end_row)
{
    for (size_t r=start_row; r <= end_row; ++r)
        if (df->data[r][col].type == DATACELL_DOUBLE)
            return &df->data[r][col];
    return NULL;
}

void df_col_range_fill(DataFrame *df, size_t col, size_t start_row, size_t end_row)
{
    DataCell *next_cell, *prev_cell = next_cell = NULL;
    for (size_t r=start_row; r <= end_row; ++r)
    {
        switch (df->data[r][col].type)
        {
            case DATACELL_DOUBLE:
                next_cell = NULL, prev_cell = &df->data[r][col];
                break;

            case DATACELL_NAN:
                if (prev_cell && (next_cell || (next_cell = find_next_cell(df, col, r+1,end_row))))
                {
                    assert(prev_cell->type == next_cell->type && next_cell->type == DATACELL_DOUBLE);
                    df->data[r][col].as_double = (prev_cell->as_double + next_cell->as_double) / 2;
                    df->data[r][col].type = DATACELL_DOUBLE;
                }
                break;

            default:
                break;
        }
    }
}

void df_range_fill(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col)
{
    for (size_t c=start_col; c <= end_col; ++c)
        df_col_range_fill(df, c, start_row, end_row);
}

void df_col_fill(DataFrame *df, size_t col)
{
    df_col_range_fill(df, col, 0, df->rows-1);
}

void df_fill(DataFrame *df)
{
    df_range_fill(df, 0,0, df->rows-1,df->cols);
}

void df_range_fill_const(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col, double value)
{
    for (size_t r=start_row; r <= end_row; ++r)
        for (size_t c=start_col; c <= end_col; ++c)
            df->data[r][c].type = DATACELL_DOUBLE, df->data[r][c].as_double = value;
}

void df_col_fill_const(DataFrame *df, size_t col, double value)
{
    df_range_fill_const(df, 0,col, df->rows-1,col, value);
}

void df_fill_const(DataFrame *df, double value)
{
    df_range_fill_const(df, 0,0, df->rows-1,df->cols-1, value);
}

static const char *skip_spaces(const char *s)
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
        for (size_t c=start_col; c <= end_col; ++c)
        {
            if (df->data[r][c].type != DATACELL_STR)
                continue;
            char *rest;
            double val = strtod(df->data[r][c].as_str, &rest);
            // TODO: error checking (HUGE_VAL, ERANGE errno)
            if (rest != df->data[r][c].as_str && (strictness == DATACELL_CONVERT_LAX || !*skip_spaces(rest)))
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

void df_to_double(DataFrame *df, enum DataCell_ConvertStrictness strictness)
{
    df_range_to_double(df, 0, 0, df->rows-1, df->cols-1, strictness);
}

void df_col_range_min_max(const DataFrame *df, size_t col, size_t start_row, size_t end_row, double *min, double *max)
{
#ifdef INFINITY
    *max = -INFINITY, *min = INFINITY;
#else
    *max = -HUGE_VAL, *min = HUGE_VAL;
#endif
    for (size_t r=start_row; r <= end_row; ++r)
    {
        if (df->data[r][col].type != DATACELL_DOUBLE)
            continue;
        if (*max < df->data[r][col].as_double)
            *max = df->data[r][col].as_double;
        if (df->data[r][col].as_double < *min)
            *min = df->data[r][col].as_double;
    }
}

void df_col_min_max(const DataFrame *df, size_t col, double *min, double *max)
{
    df_col_range_min_max(df, col, 0,df->rows-1, min, max);
}

void df_col_range_normalize(DataFrame *df, size_t col, size_t start_row, size_t end_row)
{
    double min, max;
    df_col_range_min_max(df, col, start_row, end_row, &min, &max);
    double delta = max-min;
    for (size_t r=start_row; r <= end_row; ++r)
    {
        if (df->data[r][col].type == DATACELL_DOUBLE)
        {
            df->data[r][col].as_double = uai_normalized_value(df->data[r][col].as_double, min, delta);
            df->data[r][col].min = min, df->data[r][col].delta = delta;
        }
    }
}

void df_range_normalize(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col)
{
    for (size_t c=start_col; c <= end_col; ++c)
        df_col_range_normalize(df, c, start_row, end_row);
}

void df_col_normalize(DataFrame *df, size_t col)
{
    df_col_range_normalize(df, col, 0, df->rows-1);
}

void df_normalize(DataFrame *df)
{
    df_range_normalize(df, 0,0, df->rows-1,df->cols-1);
}

void df_col_range_denormalize(DataFrame *df, size_t col, size_t start_row, size_t end_row)
{
    for (size_t r=start_row; r <= end_row; ++r)
    {
        if (df->data[r][col].type == DATACELL_DOUBLE)
            df->data[r][col].as_double = uai_denormalize_value(df->data[r][col].as_double, df->data[r][col].min, df->data[r][col].delta);
    }
}

void df_range_denormalize(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col)
{
    for (size_t c=start_col; c <= end_col; ++c)
        df_col_range_denormalize(df, c, start_row, end_row);
}

void df_col_denormalize(DataFrame *df, size_t col)
{
    df_col_range_denormalize(df, col, 0, df->rows-1);
}

void df_denormalize(DataFrame *df)
{
    df_range_denormalize(df, 0,0, df->rows-1,df->cols-1);
}

int df_cell_compare(const DataCell *a, const DataCell *b)
{
    size_t type_diff = b->type - a->type;
    if (type_diff)
        return type_diff;
    switch (a->type)
    {
        case DATACELL_NAN:
            return 0;
        case DATACELL_STR:
            return strcmp(a->as_str, b->as_str);
        case DATACELL_DOUBLE:
            return b->as_double - a->as_double;
    }
    assert(0);
}

void df_range_add_labels(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col)
{
    assert(start_row <= end_row && end_row < df->rows);
    assert(start_col <= end_col && end_col < df->cols);

    static size_t latest_label = 0;

    for (size_t r=start_row; r <= end_row; ++r)
        for (size_t c=start_col; c <= end_col; ++c)
        {
            if (df->data[r][c].type != DATACELL_STR)
                continue;
            ++latest_label;
            for (size_t dr=r; dr <= end_row; ++dr)
                for (size_t dc=c; dc <= end_col; ++dc)
                {
                    DataCell *cell_a=&df->data[r][c], *cell_b=&df->data[dr][dc];
                    if (cell_a != cell_b && !df_cell_compare(cell_a, cell_b))
                        cell_b->type = DATACELL_DOUBLE, cell_b->as_double = latest_label;
                }
            df->data[r][c].type = DATACELL_DOUBLE, df->data[r][c].as_double = latest_label;
        }
}

void df_col_add_labels(DataFrame *df, size_t col)
{
    df_range_add_labels(df, 0,col, df->rows-1,col);
}

void df_add_labels(DataFrame *df)
{
    df_range_add_labels(df, 0,0, df->rows-1,df->cols-1);
}

void df_swap_rows(DataFrame *df, size_t row1, size_t row2)
{
    for (size_t c=0; c<df->cols; ++c)
    {
        DataCell tmp = df->data[row2][c];
        df->data[row2][c] = df->data[row1][c], df->data[row1][c] = tmp;
    }
}

void df_shuffle_rows(DataFrame *df)
{
    for (size_t i = 0; i < df->rows - 1; i++)
    {
        size_t j = i + rand() / (RAND_MAX / (df->rows - i) + 1);
        df_swap_rows(df, i, j);
    }
}

UAI_Status df_prepend_cols(DataFrame *df, size_t new_cols)
{
    static_assert(DATACELL_NAN == 0, "DATACELL_NAN is not first in the DataCellType enum");

    bool had_header = !!df->header;
    df_set_header(df, false);

    UAI_Status ret;

    size_t num_cols = df->cols + new_cols;
    DataCell *cellbuf = calloc(df->rows * num_cols, sizeof *cellbuf);
    if (!cellbuf)
    {
        ret = UAI_ERRNO;
        goto end;
    }

    for (size_t r=0; r < df->rows; ++r)
    {
        for (size_t c=0; c < df->cols; ++c)
            cellbuf[r * num_cols + c + new_cols] = df->data[r][c];
        df->data[r] = cellbuf + r * num_cols;
    }

    free(df->cellbuf);
    df->cellbuf = cellbuf;
    df->cols = num_cols;

    ret = UAI_OK;
end:
    df_set_header(df, had_header);
    return ret;
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
