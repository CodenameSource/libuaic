#pragma once

#include "data.h"

typedef struct
{
    double *betas;
    size_t betas_size;
} LinearRegressor;

UAI_Status lr_fit(LinearRegressor *reg, DataFrame *X, DataFrame *Y, size_t epochs, double learning_rate);

double lr_predict(LinearRegressor *reg, const DataCell *x, size_t x_size);

void lr_destroy(LinearRegressor *reg);