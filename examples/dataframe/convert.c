#include <stdio.h>
#include <libgen.h>

#include "../../include/uai/data.h"

#include "../common.h"

#define UAI_MUST(status) assert(!(status));

int main()
{
    setup_cwd(__FILE__);

    DataFrame df = {0};
    UAI_MUST(df_load_csv(&df, "csv/student_data.csv", ','));
    df_set_header(&df, true);

    df_all_to_double(&df, DATA_CELL_CONVERT_LAX);

    df_destroy(&df);
}
