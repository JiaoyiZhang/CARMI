#ifndef BIN_TYPE_H
#define BIN_TYPE_H

#include "../../../params.h"
#include "trainModel/binary_search_model.h"
#include <vector>
using namespace std;

class BSType
{
public:
    BSType() = default;
    BSType(int c)
    {
        flagNumber = (3 << 24) + c;
        model = BinarySearchModel(c);
    }

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;           // 4c Byte + 4
    BinarySearchModel model; // 8c + 4
};

#endif