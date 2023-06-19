#include <assert.h>
#include <stdio.h>
#include <libgen.h>

#include "../../include/uai/data.h"

#include "../common.h"


int main()
{
    setup_cwd();

    DataFrame df={0}, df2={0};
    UAI_MUST(df_load_csv(&df, "csv/distances.csv", ','));
    df_set_header(&df, true);

    UAI_MUST(df_copy(&df, &df2));

    df_to_double(&df, DATACELL_CONVERT_LAX);
    assert(df.data[0][1].type == DATACELL_DOUBLE);
    assert(df.data[0][0].type == DATACELL_NAN);
    assert(df.data[2][1].type == DATACELL_DOUBLE);
    assert(df.data[3][1].type == DATACELL_DOUBLE);
    printf("Distance Sofia-Pernik: %.2f (km)\n", df.data[0][1].as_double);

    df_col_to_double(&df2, 1, DATACELL_CONVERT_STRICT);
    assert(df2.data[0][1].type == DATACELL_NAN);
    assert(df2.data[0][0].type == DATACELL_STR);
    assert(df2.data[2][1].type == DATACELL_DOUBLE);
    assert(df2.data[3][1].type == DATACELL_DOUBLE);
    puts("Distance Sofia-Pernik: nan");

    df_destroy(&df);
    df_destroy(&df2);
}
