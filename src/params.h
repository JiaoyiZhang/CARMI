#ifndef PARAMS
#define PARAMS
#include "leafNode/array.h"
#include "leafNode/gapped_array.h"

#define LEAF_NODE_TYPE ArrayNode
// #define LEAF_NODE_TYPE GappedArray

extern int kLeafNodeID;
extern int kInnerNodeID;

extern const int kMaxEpoch = 500;
extern const double kLearningRate = 0.0001;
// const int kNeuronNumber = 8;
extern const int kNeuronNumber = 1;

extern const int kThreshold = 400;
extern const double kDensity = 0.5;
extern const int kMaxKeyNum = 10000;

#endif