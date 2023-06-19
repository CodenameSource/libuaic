#pragma once

#include "data.h"

typedef struct
{
    double *betas;
    size_t betas_size;
} LinearRegressor;

UAI_Status lr_fit(LinearRegressor *reg, DataFrame *X, DataFrame *Y, size_t epochs, double learning_rate);

UAI_Status lr_predict(LinearRegressor *reg, const DataFrame *X, DataFrame *Y, size_t y_col);
