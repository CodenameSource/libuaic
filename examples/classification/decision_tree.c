#include <stdlib.h>
#include <stdio.h>

#include "uai/data.h"
#include "uai/decision_tree.h"
#include "../common.h"


#define UAI_MUST(status) assert(!(status));
int main() {
    setup_cwd();

    DataFrame X = {0}, Y;

    UAI_MUST(df_load_csv(&X, "csv/student_data3.csv", ','));

    df_set_header(&X, true);
    df_range_add_labels(&X, 0, 3, X.rows-1, 3);
    df_to_double(&X, DATACELL_CONVERT_LAX);

    UAI_MUST(df_create_vsplit(&X, &Y, 1, DATAFRAME_SAMPLE_SEQ));
    UAI_MUST(df_export_csv(&X, "out/tmp01.csv", ','));
    UAI_MUST(df_export_csv(&Y, "out/tmp02.csv", ','));

    DecisionTree *dt = dt_init();

    dt_fit(dt, &X, &Y, 10, 1);


    df_destroy(&X);
    df_destroy(&Y);
}
