#include "common.h"

typedef struct
{
    const char *str;
} DataCell;

/**
 * @brief Two-dimensional, potentially heterogeneous tabular data.
 **/
typedef struct
{
    size_t rows, cols;
    DataCell **data;

    size_t strbuf_size;
    char *strbuf;
    DataCell *cellbuf;
} DataFrame;

UAI_Status df_load_csv(DataFrame *df, const char *filename, char sep);
