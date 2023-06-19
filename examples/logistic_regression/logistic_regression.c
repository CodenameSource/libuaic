#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <time.h>

#include "../../include/uai/data.h"
#include "../../include/uai/logistic_regression.h"

#include "../common.h"

int main()
{
    setup_cwd();

    DataFrame test = {0}, X = {0}, Y = {0};
    UAI_MUST(df_load_csv(&test, "csv/student4.csv", ','));
    df_set_header(&test, true);

    df_to_double(&test, DATACELL_CONVERT_LAX);


    UAI_MUST(df_create_vsplit(&test, &Y, 1, DATAFRAME_SAMPLE_SEQ));
    UAI_MUST(df_create_vsplit(&test, &X, 4, DATAFRAME_SAMPLE_SEQ));



    LogisticRegressor *reg = lg_create();
    lg_fit(reg, &X, &Y, 10000, 0.00002);

    for (size_t r=0; r < Y.rows; ++r)
        printf("Prediction: %lf for %zu row\n", lg_predict(reg, X.data[r], X.cols), r);

    df_destroy(&test);
    df_destroy(&Y);
    df_destroy(&X);
    lg_destroy(reg);
}
