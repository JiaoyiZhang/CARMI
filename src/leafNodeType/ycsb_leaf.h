#ifndef YCSB_LEAF_H
#define YCSB_LEAF_H

#include <iostream>
#include <vector>
using namespace std;

class YCSBLeaf
{
public:
    // YCSBLeaf() = default;
    YCSBLeaf()
    {
        flagNumber = (10 << 24) + 0;
        error = 0;
        m_left = -1;
    }
    inline int UpdateError(const int start_idx, const int size);
    inline void SetDataset(const int start_idx, const int size);
    
    inline void Train(const int start_idx, const int size);
    inline int Predict(double key);

    int flagNumber; // 4 Byte (flag + 0)

    int m_left; // the left boundary of the leaf node in the global array
    int error;  // the boundary of binary search

    float theta1;
    float theta2;
    float tmpMember;
    pair<float, float> tmppp[5];
};

#endif // !YCSB_LEAF_H