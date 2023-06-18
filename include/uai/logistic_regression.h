#include "data.h"

struct LogisticRegressor {
    double *beta;
    size_t beta_size;
};

struct LogisticRegressor *init();

void lg_fit(struct LogisticRegressor *regressor, DataFrame *X, DataFrame *Y, size_t epochs, double learning_rate, long seed);

double lg_predict(struct LogisticRegressor *regressor, DataCell *X, size_t data_size);

void purge(struct LogisticRegressor *regressor);