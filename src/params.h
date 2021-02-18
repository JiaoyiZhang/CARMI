#ifndef PARAMS
#define PARAMS

#define ZIPFIAN 1
#define PARAM_ZIPFIAN 0.99

bool kIsYCSB = false;

extern int kLeafNodeID;  // for static structure
extern int kInnerNodeID; // for static structure
int childNum;     // for static structure

int kThreshold = 2;  // used to initialize a leaf node
int kMaxKeyNum = 16; // used in construction
double kRate = 0.4;  // totalCost = space * kRate + time

extern const double kDensity = 0.5;        // the density of gap in gapped array
extern const double kExpansionScale = 1.5; // scale of array expansion

extern unsigned int entireChildNumber;
extern unsigned int nowChildNumber;

#endif