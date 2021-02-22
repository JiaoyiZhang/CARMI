#ifndef LR_TYPE_H
#define LR_TYPE_H

#include "../../../params.h"

#include "trainModel/linear_regression.h"
#include <vector>
#include <fstream>
using namespace std;

class LRType
{
public:
    LRType() = default;
    LRType(int c)
    {
        flagNumber = (0 << 24) + c;
    }

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;          // 4 Byte
    LinearRegression model; // 20 Byte
};

#endif