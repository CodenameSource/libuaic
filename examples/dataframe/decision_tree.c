#include <stdlib.h>
#include <stdio.h>

#include "../../include/uai/data.h"
#include "../../include/uai/decision_tree.h"
#include "../common.h"


#define UAI_MUST(status) assert(!(status));
int main() {
    setup_cwd();

    DataFrame df = {0};

    UAI_MUST(df_load_csv(&df, "csv/student_data.csv", ','));

    df_set_header(&df, true);
    df_to_double(&df, DATACELL_CONVERT_LAX);

    DecisionTree *dt = dt_init();



    df_destroy(&df);
}