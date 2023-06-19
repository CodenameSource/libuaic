#include <stdio.h>
#include <libgen.h>

#include "../../include/uai/data.h"

#include "../common.h"


int main()
{
    setup_cwd();

    DataFrame df = {0};
    UAI_MUST(df_load_csv(&df, "csv/test.csv", ','));
    df_set_header(&df, true);
    UAI_MUST(df_prepend_cols(&df, 3));
    df_col_fill_const(&df, 0, 10);
    UAI_MUST(df_export_csv(&df, "out/test_prepended.csv", ','));
    df_destroy(&df);
}
