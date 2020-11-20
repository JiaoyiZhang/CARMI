#ifndef PARAMS
#define PARAMS

// 0:binary search; 1:exponential search
extern const int kSearchMethod = 0; 

extern int kLeafNodeID;
extern int kInnerNodeID;
 
extern const int kMaxEpoch = 20;  // used in nn
extern const double kLearningRate = 0.0000001;  // used in nn
extern int kNeuronNumber;  // used in nn

extern int kThreshold;  // used to initialize a leaf node
extern int kMaxKeyNum;  // used in construction
extern int kMaxSpace;  // used in construction
extern double kRate;  // totalCost = space * kRate + time
extern const double kDensity = 0.5;  // the density of gap in gapped array
extern const double kReadWriteRate = 0.9;  // readTimes / (readTimes + writeTimes)

extern vector<vector<pair<double, double>>> entireDataset;

#endif