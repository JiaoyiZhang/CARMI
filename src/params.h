#ifndef PARAMS
#define PARAMS

#define ZIPFIAN 0
#define PARAM_ZIPFIAN 1.25

bool kIsYCSB = false;

extern int kLeafNodeID;  // for static structure
extern int kInnerNodeID; // for static structure
int childNum = 3907;     // for static structure

extern const int kMaxEpoch = 20;               // used in nn
extern const double kLearningRate = 0.0000001; // used in nn
int kNeuronNumber = 8;                         // used in nn

int kThreshold = 2;  // used to initialize a leaf node
int kMaxKeyNum = 16; // used in construction
double kRate = 0.4;  // totalCost = space * kRate + time

extern const double kDensity = 0.5;        // the density of gap in gapped array
extern const double kExpansionScale = 1.5; // scale of array expansion

extern unsigned int entireChildNumber;
extern unsigned int nowChildNumber;

#endif