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
    inline int Predict(double key)
    {
        // return the predicted idx in the leaf node
        int size = (flagNumber & 0x00FFFFFF);
        int p = (theta1 * key + theta2) * size;
        if (p < 0)
            p = 0;
        else if (p >= size)
            p = size - 1;
        return p;
    }

    int flagNumber; // 4 Byte (flag + 0)

    int m_left; // the left boundary of the leaf node in the global array
    int error;  // the boundary of binary search

    float theta1;
    float theta2;
    float tmpMember;
    pair<float, float> tmppp[5];
};

#endif // !YCSB_LEAF_H