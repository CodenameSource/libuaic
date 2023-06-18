#include "../include/uai/linear_regression.h"

#include <assert.h>

// TODO: swap w/ real def
double dot_dataframe(const double *a, const DataCell *b, size_t n);

void lr_fit(LinearRegressor *reg, DataFrame *X, DataFrame *Y)
{
}

void lr_predict(LinearRegressor *reg, const DataFrame *X, DataFrame *Y, size_t y_col)
{
    assert(reg->betas_size && X->cols == reg->betas_size);
    assert(Y->rows == X->rows);

    DataFrame prepended_df = {0};
    df_copy(X, &prepended_df);
    df_prepend_cols(&prepended_df, 1);
    df_col_fill_const(&prepended_df, 0, 1.0);

    for (size_t r=0; r<Y->rows; ++r)
        Y->data[r][y_col] = (DataCell){ .type=DATACELL_DOUBLE, .as_double=dot_dataframe(reg->betas, X->data[r], X->cols) };

    df_destroy(&prepended_df);
}
