#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <time.h>

#include "../../include/uai/data.h"
#include "../../include/uai/linear_regression.h"

#include "../common.h"


int main()
{
    setup_cwd();

    DataFrame test = {0}, X = {0}, Y = {0};
    UAI_MUST(df_load_csv(&test, "csv/houses.csv", ','));
    df_set_header(&test, true);

    df_to_double(&test, DATACELL_CONVERT_STRICT);
    df_normalize(&test);

    srand(time(NULL));

    UAI_MUST(df_create_vsplit(&test, &Y, 1, DATAFRAME_SAMPLE_SEQ));
    UAI_MUST(df_create_vsplit(&test, &X, 4, DATAFRAME_SAMPLE_SEQ));

    LinearRegressor *reg = lr_init();
    lr_fit(&reg, &X, &Y, 5000, 0.14);

    for (size_t r=0; r < Y.rows; ++r)
        printf("Prediction: %lf for %zu row\n", lr_predict(&reg, X.data[r], X.cols), r);


    lr_destroy(&reg);

    df_destroy(&test);
    df_destroy(&X);
    df_destroy(&Y);
}
