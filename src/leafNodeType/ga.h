#ifndef GA_H
#define GA_H

#include <iostream>
#include <vector>
#include "../params.h"
using namespace std;

class GappedArrayType
{
public:
    GappedArrayType() = default;
    GappedArrayType(int cap)
    {
        flagNumber = (9 << 24) + 0;
        error = 0;
        density = kDensity;
        capacity = cap;
        maxIndex = -2;
        m_left = -1;
    }
    inline int UpdateError(const vector<pair<double, double>> &dataset);
    inline int UpdateError(const int start_idx, const int size);
    inline void SetDataset(const vector<pair<double, double>> &dataset, int cap);
    inline void SetDataset(const int left, const int size, int cap);
    inline void SetDataset(const int start_idx, const int size);
    void Train(const vector<pair<double, double>> &dataset);
    void Train(const int start_idx, const int size);
    int Predict(double key);

    int flagNumber; // 4 Byte (flag + 0)

    int m_left;   // the left boundary of the leaf node in the global array
    int capacity; // the current maximum capacity of the leaf node

    int maxIndex;  // tht index of the last one
    int error;     // the boundary of binary search
    float density; // the maximum density of the leaf node data

    float theta1;
    float theta2;
    pair<float, float> tmppp[4];
};

#endif // !GA_H