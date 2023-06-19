#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "../include/uai/decision_tree.h"

// TODO: Fix this unoptimized memory munching mess
// TODO: Get some sleep and preferably a life

struct fitBuffer {
    DataCell **left_idx, **right_idx, **y_left, **y_right;
    size_t left_size, right_size, unique_size;

    double *unique;
};

// Helper functions

struct Node *create_node(struct Node *left, struct Node *right, size_t value, size_t feature_idx, double threshold);

double entropy(struct fitBuffer *buffer, DataCell **Y, size_t size);

void split(struct fitBuffer *buffer, DataCell **X, DataCell **Y, size_t data_size, size_t feature_idx, double threshold);

void get_unique(struct fitBuffer *buffer, DataCell **data, size_t data_size, size_t feature_idx);

double get_most_common(struct fitBuffer *buffer, DataCell **data, size_t data_size);

double info_gain(struct fitBuffer *buffer, DataCell **X, DataCell **Y, size_t data_size, size_t feature_idx, double threshold);

void best_split(struct fitBuffer *buffer, DataCell **X, DataCell **Y, size_t data_size, size_t feature_cnt, size_t *feature_idx, double *threshold);

struct Node *build_tree(struct fitBuffer *buffer, DataCell **X, DataCell **Y, size_t data_size, size_t feature_cnt, size_t depth, size_t max_depth, size_t min_samples_split);

size_t traverse_tree(DataCell *x, struct Node *node);

void purge_tree(struct Node *node);

DecisionTree *dt_init() {
    DecisionTree *tree = malloc(sizeof(DecisionTree));
    if(!tree) {
        printf("Error allocating memory for DecisionTree\n");
        return NULL;
    }

    tree->root = NULL;

    return tree;
}

void dt_fit(DecisionTree *tree, DataFrame *X, DataFrame *Y, size_t max_depth, size_t min_samples_split) {
    struct fitBuffer buffer;
    buffer.left_idx = malloc(sizeof(DataCell *) * (X->rows+1));
    buffer.right_idx = malloc(sizeof(DataCell *) * (X->rows+1));
    buffer.y_left = malloc(sizeof(DataCell *) * (X->rows+1));
    buffer.y_right = malloc(sizeof(DataCell *) * (X->rows+1));
    buffer.unique = malloc(sizeof(double) * (X->rows+1));

    if(!buffer.left_idx || !buffer.right_idx || !buffer.y_left || !buffer.y_right || !buffer.unique) {
        printf("Error allocating memory for fit buffer\n");
        return;
    }

    buffer.left_size = 0;
    buffer.right_size = 0;
    buffer.unique_size = 0;

    tree->root = build_tree(&buffer, X->data, Y->data, X->rows, X->cols, 0, max_depth, min_samples_split);

    free(buffer.left_idx);
    free(buffer.right_idx);
    free(buffer.y_left);
    free(buffer.y_right);
    free(buffer.unique);
}

size_t dt_predict(DecisionTree *tree, DataCell *x) {
    return traverse_tree(x, tree->root);
}

void dt_purge(DecisionTree *tree) {
    purge_tree(tree->root);
    free(tree);
}


struct Node *create_node(struct Node *left, struct Node *right, size_t value, size_t feature_idx, double threshold) {
    struct Node *node = malloc(sizeof(struct Node));
    if(!node)
        return NULL;

    node->left = left;
    node->right = right;
    node->class_prediction = value;
    node->feature_idx = feature_idx;
    node->threshold = threshold;
    return node;
}

size_t get_occurrences(DataCell **Y, double n, size_t size) {
    size_t occurences = 0;
    for(size_t i = 0;i < size;i++)
        if(Y[i][0].as_double == n)
            occurences++;

    return occurences;
}

double entropy(struct fitBuffer *buffer, DataCell **Y, size_t size)  {
    get_unique(buffer, Y, size, 0);

    double entropy = 0, tmp = 0;
    for(size_t i = 0;i < buffer->unique_size;i++) {
        tmp = (double)get_occurrences(Y, buffer->unique[i], size) / size;
        tmp = tmp * log2(tmp);
        if(!isnan(tmp))
            entropy -= tmp;
    }

    return entropy;
}

void split(struct fitBuffer *buffer, DataCell **X, DataCell **Y, size_t data_size, size_t feature_idx, double threshold) {
    buffer->left_size = 0;
    buffer->right_size = 0;

    for(size_t i = 0;i < data_size;i++){
        if(X[i][feature_idx].as_double >= threshold) {
            if(Y)
                buffer->y_left[buffer->left_size] = Y[i];
            buffer->left_idx[buffer->left_size] = X[i];
            buffer->left_size++;
        }
        else {
            if(Y)
                buffer->y_right[buffer->right_size] = Y[i];
            buffer->right_idx[buffer->right_size] = X[i];
            buffer->right_size++;
        }
    }
}

void get_unique(struct fitBuffer *buffer, DataCell **data, size_t data_size, size_t feature_idx) {
    buffer->unique_size = 0;

    bool unique = true;

    for(size_t i = 0;i < data_size;i++) {
        unique = true;
        for(size_t j = 0;unique && j < buffer->unique_size;j++) {
            if(data[i][feature_idx].as_double == buffer->unique[j]) //TODO: Optimize finding unique values
                unique = false;
        }
        if(unique) {
            buffer->unique[buffer->unique_size] = data[i][feature_idx].as_double;
            buffer->unique_size++;
        }
    }
}

double get_most_common(struct fitBuffer *buffer, DataCell **data, size_t data_size) {
    get_unique(buffer, data, data_size, 0);

    size_t max_occ = 0, curr_occ, max = 0;
    for(size_t i = 0;i < buffer->unique_size;i++) {
        curr_occ = get_occurrences(data, buffer->unique[i], data_size);
        if(curr_occ > max_occ) {
            max = buffer->unique[i];
            max_occ = curr_occ;
        }
    }

    return max;
}

double info_gain(struct fitBuffer *buffer, DataCell **X, DataCell **Y, size_t data_size, size_t feature_idx, double threshold) {
    double parent_entropy = entropy(buffer, Y, data_size);

    split(buffer, X, Y, data_size, feature_idx, threshold);

    if(buffer->left_size == 0 || buffer->right_size == 0)
        return 0;

    double left_entropy = entropy(buffer, buffer->left_idx, buffer->left_size), \
    right_entropy = entropy(buffer, buffer->right_idx, buffer->right_size);

    return parent_entropy - (left_entropy * buffer->left_size + right_entropy * buffer->right_size) / data_size;
}

void best_split(struct fitBuffer *buffer, DataCell **X, DataCell **Y, size_t data_size, size_t feature_cnt, size_t *feature_idx, double *threshold) {
    double max_info_gain = -1;

    for(size_t i = 0;i < feature_cnt;i++) {
        get_unique(buffer, X, data_size, i);
        for(size_t j = 0;j < buffer->unique_size;j++) {
            double gain = info_gain(buffer, X, Y,  data_size, i, buffer->unique[j]);
            if(gain > max_info_gain) {
                max_info_gain = gain;
                *feature_idx = i;
                *threshold = buffer->unique[j];
            }
        }
    }
}

struct Node *build_tree(struct fitBuffer *buffer, DataCell **X, DataCell **Y, size_t data_size, size_t feature_cnt, size_t depth, size_t max_depth, size_t min_samples_split) {
    get_unique(buffer, Y, data_size, 0);

    if(depth >= max_depth || data_size < min_samples_split || buffer->unique_size == 1) {
        return create_node(NULL, NULL, get_most_common(buffer, Y, data_size), 0, 0);
    }

    size_t best_feature;
    double best_threshold;

    best_split(buffer, X, Y, data_size, feature_cnt, &best_feature, &best_threshold);

    split(buffer, X, Y, data_size, best_feature, best_threshold);

    struct Node *node = create_node(NULL, NULL, 0, best_feature, best_threshold);
    if(!node)
        return NULL;

    node->left = build_tree(buffer, buffer->left_idx, buffer->y_left, buffer->left_size, feature_cnt, depth + 1, max_depth, min_samples_split);
    if(!node->left)
        return NULL;
    node->right = build_tree(buffer, buffer->right_idx, buffer->y_right, buffer->right_size, feature_cnt, depth + 1, max_depth, min_samples_split);
    if(!node->right)
        return NULL;

    return node;
}

size_t traverse_tree(DataCell *x, struct Node *node) {
    while(node->left && node->right) {
        if(x[node->feature_idx].as_double <= node->threshold)
            node = node->left;
        else
            node = node->right;
    }

    return node->class_prediction;
}

void purge_tree(struct Node *node) {
    if(node->left)
        purge_tree(node->left);
    if(node->right)
        purge_tree(node->right);

    free(node);
}
