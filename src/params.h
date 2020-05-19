#ifndef PARAMS
#define PARAMS
#include "leafNode/array.h"
#include "leafNode/gapped_array.h"

#define LEAF_NODE_TYPE ArrayNode
// #define LEAF_NODE_TYPE GappedArray

// 0:binary search; 1:exponential search
extern const int kSearchMethod = 0; 

extern int kLeafNodeID;
extern int kInnerNodeID;

extern const int kMaxEpoch = 500;
extern const double kLearningRate = 0.0001;
// const int kNeuronNumber = 8;
extern int kNeuronNumber;

extern const int kThreshold = 80000;
extern const double kDensity = 0.5;
extern const int kMaxKeyNum = 100000;

#endif