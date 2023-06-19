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

    DataFrame test = {0};
    UAI_MUST(df_load_csv(&test, "csv/houses.csv", ','));
    df_set_header(&test, true);
    df_to_double(&test, DATACELL_CONVERT_STRICT);
    df_fill(&test);
    df_normalize(&test);

    srand(time(NULL));

    DataFrame train = {0};
    UAI_MUST(df_create_hsplit(&test, &train, test.rows / 5, DATAFRAME_SAMPLE_RAND));

    DataFrame test_y = {0};
    UAI_MUST(df_create_vsplit(&test, &test_y, 1, DATAFRAME_SAMPLE_SEQ));

    DataFrame train_y = {0};
    UAI_MUST(df_create_vsplit(&train, &train_y, 1, DATAFRAME_SAMPLE_SEQ));

    LinearRegressor reg = {0};
    lr_fit(&reg, &train, &train_y, 5000, 0.14);

    lr_predict(&reg, &test, &test_y, 0);

    df_export_csv(&test, "out/houses_linear_regression.csv", ',');
    df_export_csv(&test_y, "out/houses_linear_regression_y.csv", ',');

    df_destroy(&test);
    df_destroy(&train);
    df_destroy(&test_y);
    df_destroy(&train_y);
}
