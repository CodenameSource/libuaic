#pragma once

#include "data.h"

struct Node {
    struct Node *left, *right;

    double threshold;
    size_t feature_idx;
    double class_prediction;
};

typedef struct DecisionTree {
    struct Node *root;
    //size_t depth;
}DecisionTree;

struct DecisionTree *dt_init();

void dt_fit(struct DecisionTree *tree, DataFrame *X, DataFrame *Y, size_t max_depth, size_t min_samples_split);

size_t dt_predict(struct DecisionTree *tree, DataCell *x);

void dt_purge(struct DecisionTree *tree);