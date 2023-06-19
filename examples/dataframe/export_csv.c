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

    df.data[0][0].type = DATACELL_DOUBLE, df.data[0][0].as_double = 3.1415;

    UAI_MUST(df_export_csv(&df, "out/test.csv", ','));

    df_destroy(&df);
}
