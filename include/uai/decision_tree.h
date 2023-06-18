#include "data.h"

struct Node {

    size_t class_prediction;
};

struct DecisionTree {
    struct Node *root;
    size_t depth;
};

struct DecisionTree *init();

void dt_fit();

void dt_purge(struct DecisionTree *tree);