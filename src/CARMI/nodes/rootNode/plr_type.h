#ifndef NN_TYPE_H
#define NN_TYPE_H

#include "../../../params.h"
#include "trainModel/piecewiseLR.h"
#include <vector>
using namespace std;

class PLRType
{
public:
    PLRType() = default;
    PLRType(int c)
    {
        flagNumber = (1 << 24) + c;
    }

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;     // 4 Byte
    PiecewiseLR model; // 24*8+4 Byte
};

#endif