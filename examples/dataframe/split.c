#include <stdio.h>
#include <libgen.h>

#include "../../include/uai/data.h"

#include "../common.h"


int main()
{
    setup_cwd();

    DataFrame train1 = {0};
    UAI_MUST(df_load_csv(&train1, "csv/student_data.csv", ','));
    df_set_header(&train1, true);

    DataFrame train2 = {0};
    UAI_MUST(df_copy(&train1, &train2));

    DataFrame test1 = {0};
    UAI_MUST(df_create_hsplit(&train1, &test1, train1.rows / 3, DATAFRAME_SAMPLE_SEQ));
    DataFrame scores1 = {0};
    UAI_MUST(df_create_vsplit(&test1, &scores1, 1, DATAFRAME_SAMPLE_SEQ));
    UAI_MUST(df_export_csv(&train1, "out/students_train1.csv", ','));
    UAI_MUST(df_export_csv(&test1, "out/students_test1.csv", ','));
    UAI_MUST(df_export_csv(&scores1, "out/students_scores1.csv", ','));

    DataFrame test2 = {0};
    UAI_MUST(df_create_hsplit(&train2, &test2, train2.rows / 3, DATAFRAME_SAMPLE_RAND));
    DataFrame scores2 = {0};
    UAI_MUST(df_create_vsplit(&test2, &scores2, 1, DATAFRAME_SAMPLE_SEQ));
    UAI_MUST(df_export_csv(&train2, "out/students_train2.csv", ','));
    UAI_MUST(df_export_csv(&test2, "out/students_test2.csv", ','));
    UAI_MUST(df_export_csv(&scores2, "out/students_scores2.csv", ','));

    df_destroy(&train1);
    df_destroy(&test1);
    df_destroy(&scores1);
    df_destroy(&train2);
    df_destroy(&test2);
    df_destroy(&scores2);
}
