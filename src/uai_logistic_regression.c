#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "../include/uai/logistic_regression.h"

double squish(const double *beta, DataCell *x, size_t data_size);

double log_loss(double y, double y_pred);

double error(DataFrame *Y, const double *Y_pred, size_t n);

void calc_gradient(double *gradient, struct LogisticRegressor *regressor, DataFrame *X, DataFrame *Y);

void predict_dataset(double *Y_pred, struct LogisticRegressor *regressor, DataFrame *X);

void recalculate_beta(struct LogisticRegressor *regressor, double *gradient, double learning_rate);

struct LogisticRegressor *init() {
    struct LogisticRegressor *regressor = malloc(sizeof(struct LogisticRegressor));
    if(!regressor)
        return NULL;

    regressor->beta_size = 0;
    regressor->beta = NULL;

    return regressor;
}

double lg_predict(struct LogisticRegressor *regressor, DataCell *X, size_t data_size) {
    return squish(regressor->beta, X, data_size);
}

void lg_fit(struct LogisticRegressor *regressor, DataFrame *X, DataFrame *Y, size_t epochs, double learning_rate, long seed) {
    srand(seed);

    /* TODO: Replace with dataframe method
    for(size_t i = 0;i < data_size;i++) {
        X[i] = realloc(X[i], feature_cnt * sizeof(double));
        assert(X[i] != NULL);

        X[i][feature_cnt] = 1; // Appending 1 to the features matrix to simplify calculations(reduces required calculation to just the dot product of the betas and the features)
    }
    */

    regressor->beta_size = X->cols;
    regressor->beta = malloc((regressor->beta_size) * sizeof(double));
    assert(regressor->beta != NULL);

    for(size_t i = 0;i < regressor->beta_size;i++)
        regressor->beta[i] = (double) rand() / 10;

    printf("Initial betas: [");
    for(size_t i = 0;i < regressor->beta_size-1;i++) {
        printf("%lf, ", regressor->beta[i]);
    }
    printf("%lf]\n", regressor->beta[regressor->beta_size-1]);

    size_t data_size = Y->rows;

    double *Y_pred = malloc(data_size * sizeof(double));
    assert(Y_pred != NULL);

    double *gradient = malloc(regressor->beta_size * sizeof(double));
    assert(gradient != NULL);


    for(size_t i = 0;i < epochs;i++) {
        if(i % 1000 == 0)
            printf("Epoch %ld loss: %lf", i, error(Y, Y_pred, data_size));


        predict_dataset(Y_pred, regressor, X);
        calc_gradient(gradient, regressor, X, Y);
        recalculate_beta(regressor, gradient, learning_rate);
    }

    printf("Best estimate for beta: [");
    for(size_t i = 0;i < regressor->beta_size-1;i++) {
        printf("%lf, ", regressor->beta[i]);
    }
    printf("%lf]\n", regressor->beta[regressor->beta_size-1]);
}

void predict_dataset(double *Y_pred, struct LogisticRegressor *regressor, DataFrame *X) {
    assert(Y_pred != NULL);

    for(size_t i = 0;i < X->rows;i++) {
        Y_pred[i] =lg_predict(regressor, X->data[i], X->cols);
    }

}

void calc_gradient(double *gradient, struct LogisticRegressor *regressor, DataFrame *X, DataFrame *Y) {
    assert(gradient != NULL);
    assert(regressor->beta_size == X->cols);

    for(size_t i = 0;i < X->rows;i++) {
        double error = squish(regressor->beta, X->data[i], X->cols) - Y->data[i][0].as_double; // TODO: Add another check for string label or hope that the dependent variable is not a string

        for(size_t k;k < X->cols;k++) {
            gradient[i] += (error * X->data[i][k].as_double);
        }
        gradient[i] *= ((double)1 / X->cols) * gradient[i];
    }
}

void recalculate_beta(struct LogisticRegressor *regressor, double *gradient, double learning_rate) {
    for(size_t i = 0;i < regressor->beta_size;i++) {
        regressor->beta[i] = regressor->beta[i] + (gradient[i] * -learning_rate);
    }
}

double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}

double dot_dataframe(const double *a, DataCell *b, size_t n)
{
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i].as_double; // TODO: Add check for string label
    }
    return sum;
}

double squish(const double *beta, DataCell *x, size_t n)
{
    return sigmoid(dot_dataframe(beta, x, n));
}

double log_loss(double y, double y_pred)
{
    return -((y * log(y_pred)) + ((1.0 - y) * log(1.0 - y_pred)));
}

double error(DataFrame *Y, const double *Y_pred, size_t n)
{
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i)
        sum += log_loss(Y->data[i][0].as_double, Y_pred[i]);
    return (1 / n) * sum;
}

