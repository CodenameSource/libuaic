#include "../include/uai/scaling.h"

double uai_normalized_value(double value, double min, double delta)
{
    return (value - min) / delta;
}

double uai_denormalize_value(double value, double min, double delta)
{
    return (value * delta) + min;
}
