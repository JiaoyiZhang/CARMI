#ifndef PARAMS
#define PARAMS
#include "leafNode/array.h"
#include "leafNode/gapped_array.h"

// #define LEAF_NODE_TYPE ArrayNode
#define LEAF_NODE_TYPE GappedArray

// 0:binary search; 1:exponential search
extern const int kSearchMethod = 0; 

extern int kLeafNodeID;
extern int kInnerNodeID;



// 1,000,000
extern const int kMaxEpoch = 40;
extern const double kLearningRate = 0.0000001;

// //10,000
// extern const int kMaxEpoch = 500;
// extern const double kLearningRate = 0.0001;

extern int kNeuronNumber;

extern int kThreshold;
extern int kMaxKeyNum;
extern const double kDensity = 0.5;

#endif