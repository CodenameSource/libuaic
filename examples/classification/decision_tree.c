#include <stdlib.h>
#include <stdio.h>

#include "uai/data.h"
#include "uai/decision_tree.h"
#include "../common.h"


#define UAI_MUST(status) assert(!(status));
int main() {
    setup_cwd();

    DataFrame df = {0}, X, Y;

    UAI_MUST(df_load_csv(&df, "csv/student_data3.csv", ','));

    df_set_header(&df, true);
    df_range_add_labels(&df, 0, 4, df.rows-1, 4);
    df_to_double(&df, DATACELL_CONVERT_LAX);

    for(size_t i = 0;i < df.rows;i++) {
        for(size_t c = 0;c < df.cols;c++)
            assert(df.data[i][c].type != DATACELL_NAN);
    }


    UAI_MUST(df_create_vsplit(&df, &Y, 1, DATAFRAME_SAMPLE_SEQ));
    UAI_MUST(df_create_vsplit(&df, &X, 3, DATAFRAME_SAMPLE_SEQ));

    for(size_t i = 0;i < X.rows;i++) {
        for(size_t c = 0;c < X.cols;c++)
            assert(df.data[i][c].type != DATACELL_NAN);
    }

    for(size_t i = 0;i < Y.rows;i++) {
        for(size_t c = 0;c < Y.cols;c++)
            assert(df.data[i][c].type != DATACELL_NAN);
    }

    DecisionTree *dt = dt_init();

    dt_fit(dt, &X, &Y, 10, 1);


    df_destroy(&df);
    df_destroy(&X);
    df_destroy(&Y);
}