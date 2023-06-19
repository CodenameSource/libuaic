#include "../include/uai/linear_regression.h"

#include <assert.h>
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>


void recalculate_beta(LinearRegressor *reg, const double *gradient, double learning_rate);

double lr_predict(LinearRegressor *reg, const DataCell *x, size_t x_size);

void predict_dataset(double *Y_pred, LinearRegressor *reg, DataFrame *X);

double dot_dataframe(const double *a, const DataCell *b, size_t n);

double error(DataFrame *Y, const double *Y_pred, size_t n);

void calc_gradient(double *gradient, LinearRegressor *reg, DataFrame *X, DataFrame *Y);

LinearRegressor *lr_init() {
    LinearRegressor *reg = malloc(sizeof(LinearRegressor));
    if(!reg)
        return NULL;

    reg->betas = NULL;
    reg->betas_size = 0;
    return reg;
}

UAI_Status lr_fit(LinearRegressor *reg, DataFrame *X, DataFrame *y, size_t epochs, double learning_rate)
{
    assert(1 <= X->rows && X->rows == y->rows);

    reg->betas_size = X->cols+1;
    reg->betas = malloc(reg->betas_size * sizeof(double));
    if (!reg->betas)
        return UAI_ERRNO;

    for (size_t i = 0;i < reg->betas_size;i++)
        // FIXME: do we need the cast in this case??
        reg->betas[i] = (double)(rand() % 10) / 100;

    double *Y_pred = malloc(y->rows * sizeof(double));
    assert(Y_pred != NULL);

    double *gradient = malloc(reg->betas_size * sizeof(double));
    assert(gradient != NULL);

    for (size_t i = 0;i < epochs;i++) {
        predict_dataset(Y_pred, reg, X);
        if(epochs % 1000 == 0)
            printf("Epoch %ld loss: %lf\n", i, error(y, Y_pred, y->rows));

        calc_gradient(gradient, reg, X, y);
        recalculate_beta(reg, gradient, learning_rate);
    }

    free(Y_pred);
    free(gradient);
    return UAI_OK;
}

double lr_predict(LinearRegressor *reg, const DataCell *x, size_t x_size)
{
    return dot_dataframe(reg->betas, x, x_size) + reg->betas[reg->betas_size-1];
}

void lr_destroy(LinearRegressor *reg)
{
    free(reg->betas);
    free(reg);
}

void predict_dataset(double *Y_pred, LinearRegressor *reg, DataFrame *X)
{
    assert(Y_pred != NULL);

    for(size_t i = 0;i < X->rows;i++)
    {
        Y_pred[i] = lr_predict(reg, X->data[i], X->cols);
    }
}


void recalculate_beta(LinearRegressor *regressor, const double *gradient, double learning_rate)
{
    for(size_t i = 0;i < regressor->betas_size;i++)
    {
        regressor->betas[i] = regressor->betas[i] + (gradient[i] * -learning_rate);
    }
}

void calc_gradient(double *gradient, LinearRegressor *reg, DataFrame *X, DataFrame *Y)
{
    assert(gradient != NULL);
    assert(reg->betas_size-1 == X->cols);

    for(size_t i = 0;i < X->rows;i++)
    {
        double error = lr_predict(reg, X->data[i], X->cols) - Y->data[i][0].as_double; // TODO: Add another check for string label or hope that the dependent variable is not a string

        for(size_t k = 0;k < reg->betas_size-1;k++)
        {
            gradient[k] += (error * X->data[i][k].as_double);
        }
        gradient[reg->betas_size-1] += (error * 1);
    }

    for(size_t i = 0;i < reg->betas_size;i++)
    {
        gradient[i] /= X->rows;
    }
}

double error(DataFrame *Y, const double *Y_pred, size_t n)
{
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i)
        sum += (Y->data[i][0].as_double - Y_pred[i]) * (Y->data[i][0].as_double - Y_pred[i]);
    return sum;
}

double dot_dataframe(const double *a, const DataCell *b, size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i].as_double;
    }
    return sum;
} //TODO: Fix this bs
