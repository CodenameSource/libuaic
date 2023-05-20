#include <stdio.h>

#include "../../include/uai/data.h"

int main()
{
    DataFrame df = {0};
    df_load_csv(&df, "test.csv", ',');
}
