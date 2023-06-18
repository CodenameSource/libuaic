#include "common_math.h"

double dot_dataframe(const double *a, DataCell *b, size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i].as_double;
    }
    return sum;
}