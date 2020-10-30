#ifndef PARAMS
#define PARAMS


// 0:binary search; 1:exponential search
extern const int kSearchMethod = 0; 

extern int kLeafNodeID;
extern int kInnerNodeID;
 
// 1,000,000
extern const int kMaxEpoch = 20;
extern const double kLearningRate = 0.0000001;

extern int kNeuronNumber;
extern const int kAdaptiveChildNum = 40;

extern int kThreshold;
extern int kMaxKeyNum;
extern const double kDensity = 0.5;
extern const double kReadWriteRate = 0.9;  // readTimes / (readTimes + writeTimes)
extern double kRate;  // totalCost = space * kRate + time * (1 - kRate)


#endif