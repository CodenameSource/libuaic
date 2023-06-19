#include "../include/uai/linear_regression.h"

#include <assert.h>
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "./common_math.h"

static void dot_product(size_t width, size_t height, DataFrame *X, double *y_hat, DataFrame *y, size_t y_row, double *result)
{
    for (size_t i = 0; i < width; i++)
    {
        result[i] = 0.0;
        for (size_t j = 0; j < height; j++)
            result[i] += X->data[j][i].as_double * (y_hat[j] - y->data[y_row][j].as_double);
    }
}

UAI_Status lr_fit(LinearRegressor *reg, DataFrame *X, DataFrame *y, size_t epochs, double learning_rate)
{
    assert(1 <= X->rows && X->rows == y->rows);

    size_t betas_size = X->cols+1;
    double *betas = malloc(sizeof *betas * betas_size);
    if (!betas)
        return UAI_ERRNO;

    // TODO: error handling
    UAI_MUST(df_prepend_cols(X, 1));
    df_col_fill_const(X, 0, 1.0);


    for (double *beta=betas; beta < betas + betas_size; ++beta)
        // FIXME: do we need the cast in this case??
        *beta = (double)rand() / 10.;

    double *y_hat, *arrays = y_hat = malloc(sizeof *arrays * X->rows * 2);
    assert(arrays);
    double *partial_w = arrays + X->rows;
    while (epochs--)
    {
        // line equation
        for (size_t r=0; r<X->rows; ++r)
            y_hat[r] = dot_dataframe(betas, X->data[r], betas_size);

        // calculate derrivatives
        dot_product(betas_size, y->cols, X, y_hat, y, 0, partial_w);
        for (double *pw=partial_w; pw < partial_w + X->rows; ++pw)
        {
            *pw = 2 * *pw  /  X->rows;
            if (isinf(*pw))
                *pw = *pw < 0 ? -DBL_MIN : DBL_MAX;
        }

        double sum = 0;
        for (double *yh=y_hat; yh < y_hat + X->rows; ++yh)
            for (size_t r=0; r < y->rows; ++r)
            {
                // TODO: custom column
                if (y->data[r][0].type == DATACELL_DOUBLE)
                    sum += *yh - y->data[r][0].as_double;
                if (isinf(sum))
                    sum = sum < 0 ? -DBL_MIN : DBL_MAX;
            }
        double partial_d = 2 * sum / X->rows;

        // update the coefficients
        for (double *b=betas+1; b < betas + betas_size; ++b)
            for (double *pw=partial_w; pw < partial_w + X->rows; ++pw)
            {
                assert(!isnan(*b - learning_rate * *pw));
                *b -= learning_rate * *pw;
                assert(!isnan(*b));
            }
        betas[0] -= learning_rate * partial_d;
        assert(!isnan(*betas));
    }

    reg->betas = betas, reg->betas_size = betas_size;

    free(arrays);
    return UAI_OK;
}

UAI_Status lr_predict(LinearRegressor *reg, const DataFrame *X, DataFrame *Y, size_t y_col)
{
    assert(reg->betas_size && X->cols == reg->betas_size - 1);
    assert(Y->rows == X->rows);

    DataFrame prepended_df = {0};
    UAI_Status err = df_copy(X, &prepended_df);
    if (err)
        return err;
    err = df_prepend_cols(&prepended_df, 1);
    if (err)
        return err;
    df_col_fill_const(&prepended_df, 0, 1.0);

    for (size_t r=0; r<Y->rows; ++r)
        Y->data[r][y_col] = (DataCell){ .type=DATACELL_DOUBLE, .as_double=dot_dataframe(reg->betas, X->data[r], X->cols) };

    df_destroy(&prepended_df);
    return UAI_OK;
}
