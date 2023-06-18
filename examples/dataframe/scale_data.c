#include <stdio.h>
#include <libgen.h>

#include "../../include/uai/data.h"

#include "../common.h"

#define UAI_MUST(status) assert(!(status));

int main()
{
    setup_cwd();

    DataFrame df = {0};
    UAI_MUST(df_load_csv(&df, "csv/houses.csv", ','));
    df_set_header(&df, true);
    df_to_double(&df, DATACELL_CONVERT_STRICT);

    DataFrame df2 = {0};
    df_copy(&df, &df2);

    df_standardize(&df);
    UAI_MUST(df_export_csv(&df, "out/houses_standardized.csv", ','));

    df_normalize(&df2);
    UAI_MUST(df_export_csv(&df2, "out/houses_normalized.csv", ','));

    df_destroy(&df);
    df_destroy(&df2);
}
