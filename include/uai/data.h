#include <stdbool.h>
#include <stddef.h>

#include "common.h"

enum DataCellType
{
    DATACELL_NAN,
    DATACELL_STR,
    DATACELL_DOUBLE,
};

typedef struct
{
    enum DataCellType type;
    union
    {
        double as_double;
        const char *as_str;
    };
    size_t label;
} DataCell;

/**
 * @brief Two-dimensional, potentially heterogeneous tabular data.
 **/
typedef struct
{
    size_t rows, cols;
    DataCell *header, **data;

    size_t strbuf_size;
    char *strbuf;
    DataCell *cellbuf;
} DataFrame;

UAI_Status df_load_csv(DataFrame *df, const char *filename, char sep);

// TODO: docs
UAI_Status df_create(DataFrame *df, size_t rows, size_t cols);

// TODO: docs
UAI_Status df_export_csv(DataFrame *df, const char *filename, char sep);

/**
 * @brief Set whether the DataFrame has a header or not
 *
 * If enabled, the first row of the loaded data is treated as the header.
 * If disabled, it's treated as data.
 *
 **/
void df_set_header(DataFrame *df, bool value);

// TODO: docs
UAI_Status df_copy(const DataFrame *original, DataFrame *copy);

enum DataFrame_Sampling
{
    DATAFRAME_SAMPLE_SEQ,
    DATAFRAME_SAMPLE_RAND,
};

// TODO: docs
UAI_Status df_create_hsplit(DataFrame *src, DataFrame *dst, size_t take, enum DataFrame_Sampling sampling);

UAI_Status df_create_vsplit(DataFrame *src, DataFrame *dst, size_t take, enum DataFrame_Sampling sampling);

// TODO: docs
void df_col_range_fill(DataFrame *df, size_t col, size_t start_row, size_t end_row, int todo_ignored);

void df_range_fill(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col, int todo_ignored);

void df_col_fill(DataFrame *df, size_t col, int todo_ignored);

void df_fill(DataFrame *df, int todo_ignored);

// TODO: docs
void df_range_fill_const(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col, double value);

void df_col_fill_const(DataFrame *df, size_t col, double value);

void df_fill_const(DataFrame *df, double value);

enum DataCell_ConvertStrictness
{
    DATACELL_CONVERT_LAX,
    DATACELL_CONVERT_STRICT,
};

// TODO: docs
void df_range_to_double(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col, enum DataCell_ConvertStrictness strictness);

void df_col_to_double(DataFrame *df, size_t col, enum DataCell_ConvertStrictness strictness);

void df_to_double(DataFrame *df, enum DataCell_ConvertStrictness strictness);

// TODO: docs
int df_cell_compare(const DataCell *a, const DataCell *b);

// TODO: docs
void df_range_add_labels(DataFrame *df, size_t start_row, size_t start_col, size_t end_row, size_t end_col);

void df_col_add_labels(DataFrame *df, size_t col);

void df_add_labels(DataFrame *df);

// TODO: docs
void df_swap_rows(DataFrame *df, size_t row1, size_t row2);

// TODO: docs
void df_shuffle_rows(DataFrame *df);

// TODO: docs
UAI_Status df_prepend_cols(DataFrame *df, size_t new_cols);

void df_destroy(DataFrame *df);
