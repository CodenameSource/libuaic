#include <stdbool.h>

#include "common.h"

enum DataCellType
{
    DATA_CELL_STR,
    DATA_CELL_DOUBLE,
    DATA_CELL_NAN,
};

typedef struct
{
    enum DataCellType type;
    union
    {
        double as_double;
        const char *as_str;
    };
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

/**
 * @brief Set whether the DataFrame has a header or not
 *
 * If enabled, the first row of the loaded data is treated as the header.
 * If disabled, it's treated as data.
 *
 **/
void df_set_header(DataFrame *df, bool value);

// TODO: better name
enum DataCell_ConvertStrictness
{
    DATA_CELL_CONVERT_LAX,
    DATA_CELL_CONVERT_STRICT,
};

// TODO: docs
void df_all_to_double(DataFrame *df, enum DataCell_ConvertStrictness strictness);

void df_destroy(DataFrame *df);
