#ifndef HIS_TYPE_H
#define HIS_TYPE_H

#include "trainModel/histogram.h"
#include "../../../params.h"
#include <vector>
using namespace std;

class HisType
{
public:
    HisType() = default;
    HisType(int c)
    {
        flagNumber = (2 << 24) + c;
        model = HistogramModel(c);
    }

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;        // 4 Byte
    HistogramModel model; // 16+16c
};

#endif