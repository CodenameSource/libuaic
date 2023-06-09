#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "../include/uai/logistic_regression.h"

static double squish(const double *beta, DataCell *x, size_t data_size);

static double log_loss(double y, double y_pred);

static double error(DataFrame *Y, const double *Y_pred, size_t n);

static void calc_gradient(double *gradient, LogisticRegressor *regressor, DataFrame *X, DataFrame *Y);

static void predict_dataset(double *Y_pred, LogisticRegressor *regressor, DataFrame *X);

static void recalculate_beta(LogisticRegressor *regressor, double *gradient, double learning_rate);

LogisticRegressor *lg_create()
{
    LogisticRegressor *regressor = malloc(sizeof(LogisticRegressor));
    if(!regressor)
        return NULL;

    regressor->beta_size = 0;
    regressor->beta = NULL;

    return regressor;
}

double lg_predict(LogisticRegressor *regressor, DataCell *X, size_t data_size)
{
    return squish(regressor->beta, X, data_size);
}

void lg_fit(LogisticRegressor *regressor, DataFrame *X, DataFrame *Y, size_t epochs, double learning_rate)
{
    regressor->beta_size = X->cols+1;
    regressor->beta = malloc((regressor->beta_size) * sizeof(double));
    assert(regressor->beta != NULL);

    for(size_t i = 0;i < regressor->beta_size;i++)
        regressor->beta[i] = (double)(rand() % 10) / 100000;

    printf("Initial betas: [");
    for(size_t i = 0;i < regressor->beta_size-1;i++)
    {
        printf("%lf, ", regressor->beta[i]);
    }
    printf("%lf]\n", regressor->beta[regressor->beta_size-1]);

    size_t data_size = Y->rows;

    double *Y_pred = malloc(data_size * sizeof(double));
    assert(Y_pred != NULL);

    double *gradient = malloc(regressor->beta_size * sizeof(double));
    assert(gradient != NULL);


    for(size_t i = 0;i < epochs;i++)
    {
        predict_dataset(Y_pred, regressor, X);
        if(i % 1000 == 0)
            printf("Epoch %ld loss: %lf", i, error(Y, Y_pred, data_size));

        calc_gradient(gradient, regressor, X, Y);
        recalculate_beta(regressor, gradient, learning_rate);
    }

    printf("Best estimate for beta: [");
    for(size_t i = 0;i < regressor->beta_size-1;i++)
    {
        printf("%lf, ", regressor->beta[i]);
    }
    printf("%lf]\n", regressor->beta[regressor->beta_size-1]);

    free(Y_pred);
    free(gradient);
}

void lg_destroy(LogisticRegressor *regressor)
{
    free(regressor->beta);
    free(regressor);
}

static void predict_dataset(double *Y_pred, LogisticRegressor *regressor, DataFrame *X)
{
    assert(Y_pred != NULL);

    for(size_t i = 0;i < X->rows;i++)
    {
        Y_pred[i] = lg_predict(regressor, X->data[i], X->cols);
    }

}

static void calc_gradient(double *gradient, LogisticRegressor *regressor, DataFrame *X, DataFrame *Y)
{
    assert(gradient != NULL);
    assert(regressor->beta_size-1 == X->cols);

    for(size_t i = 0;i < X->rows;i++)
    {
        double error = squish(regressor->beta, X->data[i], X->cols) - Y->data[i][0].as_double; // TODO: Add another check for string label or hope that the dependent variable is not a string

        for(size_t k = 0;k < regressor->beta_size;k++)
        {
            gradient[i] += (error * X->data[i][k].as_double);
        }
        gradient[regressor->beta_size] += (error * 1);
    }

    for(size_t i = 0;i < regressor->beta_size;i++)
    {
        gradient[i] /= X->rows;
    }
}

static void recalculate_beta(LogisticRegressor *regressor, double *gradient, double learning_rate)
{
    for(size_t i = 0;i < regressor->beta_size;i++)
    {
        regressor->beta[i] = regressor->beta[i] + (gradient[i] * -learning_rate);
    }
}

static double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}

static double squish(const double *beta, DataCell *x, size_t n)
{
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        sum += beta[i] * x[i].as_double; // TODO: Add check for string label
    }
    sum += beta[n];
    return sigmoid(sum);
}

static double log_loss(double y, double y_pred)
{
    double sum;
    sum = (isnan(log(y_pred)) || isinf(log(y_pred)) ? 0: log(y_pred)) * y;
    sum += (1.0 - y) * (isnan(log(1.0 - y_pred)) || isinf(log(1.0 - y_pred)) ? 0: log(1.0 - y_pred));
    return -sum;
}

static double error(DataFrame *Y, const double *Y_pred, size_t n)
{
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i)
        sum += log_loss(Y->data[i][0].as_double, Y_pred[i]);
    return sum / n;
}

