#include <stdbool.h>
#include <stddef.h>

#include "common.h"

enum DataCellType
{
    DATACELL_STR,
    DATACELL_DOUBLE,
    DATACELL_NAN,
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

void df_destroy(DataFrame *df);
