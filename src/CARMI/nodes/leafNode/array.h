#ifndef ARRAY_H
#define ARRAY_H

#include <iostream>
#include <vector>
using namespace std;

class ArrayType
{
public:
    ArrayType() = default;
    ArrayType(int cap)
    {
        flagNumber = (8 << 24) + 0;
        error = 0;
        m_left = -1;
        m_capacity = cap;
        previousLeaf = -1;
        nextLeaf = -1;
    }
    inline int Predict(double key);

    int flagNumber; // 4 Byte (flag + 0)
    int previousLeaf;
    int nextLeaf;

    int m_left; // the left boundary of the leaf node in the global array
    int m_capacity; // the maximum capacity of this leaf node
    int error;      // the boundary of binary search

    float theta1;
    float theta2;
    pair<float, float> tmppp[4];
};

#endif // !ARRAY_H