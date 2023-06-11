#include <assert.h>
#include <stdio.h>
#include <libgen.h>

#include "../../include/uai/data.h"

#include "../common.h"

#define UAI_MUST(status) assert(!(status));

int main()
{
    setup_cwd();

    DataFrame df={0}, df2={0};
    UAI_MUST(df_load_csv(&df, "csv/distances.csv", ','));
    df_set_header(&df, true);

    UAI_MUST(df_copy(&df, &df2));

    df_all_to_double(&df, DC_CONVERT_LAX);
    assert(df.data[0][1].type == DATA_CELL_DOUBLE);
    printf("Distance Sofia-Pernik: %.2f (km)\n", df.data[0][1].as_double);

    df_all_to_double(&df2, DC_CONVERT_STRICT);
    assert(df2.data[0][1].type == DATA_CELL_DOUBLE);
    puts("Distance Sofia-Pernik: nan");

    df_destroy(&df);
    df_destroy(&df2);
}
