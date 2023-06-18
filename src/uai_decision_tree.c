#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "../include/uai/decision_tree.h"

// TODO: Fix this unoptimized memory munching mess

struct fitBuffer {
    DataCell **left_idx, **right_idx;
    size_t left_size, right_size, unique_size;

    double *entropy_buffer, *unique;
};

double entropy(double *buffer, DataFrame *Y)  {
    for(size_t i = 0;i < Y->rows;i++)
        buffer[(size_t)(Y->data[i][0].as_double)]++;// TODO: Replace this with DataCell label member instead of double to size_t conversion

    double entropy = 0;
    for(size_t i = 0;i < Y->rows;i++) {
        buffer[i] /= Y->rows;
        entropy -= buffer[i] * log2(buffer[i]);
    }

    return entropy;
}

double partial_entropy(double *buffer, DataCell **idx, size_t size)  {
    for(size_t i = 0;i < size;i++)
        buffer[(size_t)(idx[i][0].as_double)]++;// TODO: Replace this with DataCell label member instead of double to size_t conversion

    double entropy = 0;
    for(size_t i = 0;i < size;i++) {
        buffer[i] /= size;
        entropy -= buffer[i] * log2(buffer[i]);
    }

    return entropy;
}

void split(struct fitBuffer *buffer, DataFrame *X, size_t feature_idx, double threshold) {
    for(size_t i = 0;i < X->cols;i++){
        if(X->data[i][feature_idx].as_double >= threshold) {
            buffer->left_idx[buffer->left_size] = &X->data[i][feature_idx];
            buffer->left_size++;
        }
        else {
            buffer->right_idx[buffer->right_size] = &X->data[i][feature_idx];
            buffer->right_size++;
        }
    }
}

void get_unique(struct fitBuffer *buffer, DataFrame *X, size_t feature_idx) {
    buffer->unique_size = 0;

    bool unique = true;

    for(size_t i = 0;i < X->cols;i++) {
        unique = true;
        for(size_t j = 0;unique && j < X->rows;j++) {
            if(X->data[i][feature_idx].as_double == buffer->unique[j]) //TODO: Optimize finding unique values
                unique = false;
        }
        if(unique) {
            buffer->unique[buffer->unique_size] = X->data[i][feature_idx].as_double;
            buffer->unique_size++;
        }
    }
}

double info_gain(struct fitBuffer *buffer, DataFrame *X, DataFrame *Y, size_t feature_idx, double threshold) {
    double parent_entropy = entropy(buffer->entropy_buffer, Y);

    split(buffer, X, feature_idx, threshold);

    if(buffer->left_size == 0 || buffer->right_size == 0)
        return 0;

    double left_entropy = partial_entropy(buffer->entropy_buffer, buffer->left_idx, buffer->left_size), \
    right_entropy = partial_entropy(buffer->entropy_buffer, buffer->right_idx, buffer->right_size);

    return parent_entropy - (left_entropy * buffer->left_size + right_entropy * buffer->right_size) / Y->rows;
}

void best_split(struct fitBuffer *buffer, DataFrame *X, DataFrame *Y, size_t *feature_idx, double *threshold) {
    double max_info_gain = -1;

    for(size_t i = 0;i < X->cols;i++) {
        get_unique(buffer, X, i);
        for(size_t j = 0;j < buffer->unique_size;j++) {
            double gain = info_gain(buffer, X, Y, i, buffer->unique[j]);
            if(gain > max_info_gain) {
                max_info_gain = gain;
                *feature_idx = i;
                *threshold = X->data[i][j].as_double;
            }
        }
    }
}

void build_tree


// Write a decision tree
