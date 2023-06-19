#pragma once

#include "data.h"

typedef struct
{
    double *beta;
    size_t beta_size;
}  LogisticRegressor;

LogisticRegressor *lg_create();

void lg_fit(LogisticRegressor *regressor, DataFrame *X, DataFrame *Y, size_t epochs, double learning_rate);

double lg_predict(LogisticRegressor *regressor, DataCell *X, size_t data_size);

void lg_destroy(LogisticRegressor *regressor);
