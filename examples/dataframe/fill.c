#include <stdio.h>
#include <libgen.h>

#include "../../include/uai/data.h"

#include "../common.h"

int main()
{
    setup_cwd();

    DataFrame df = {0};
    UAI_MUST(df_load_csv(&df, "csv/houses.csv", ','));
    df_set_header(&df, true);
    df_to_double(&df, DATACELL_CONVERT_STRICT);

    df_fill(&df);
    df_export_csv(&df, "out/houses_filled.csv", ',');

    df_destroy(&df);
}
