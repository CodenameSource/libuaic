#pragma once

#include "data.h"

typedef struct
{
    double *betas;
    size_t betas_size;
} LinearRegressor;

void lr_fit(LinearRegressor *reg, DataFrame *X, DataFrame *Y);

void lr_predict(LinearRegressor *reg, const DataFrame *X, DataFrame *Y, size_t y_col);
