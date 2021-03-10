#ifndef HIS_H
#define HIS_H

#include <iostream>
#include <vector>
using namespace std;

class HisModel
{
public:
    HisModel() = default;
    void SetChildNumber(int c)
    {
        flagNumber = (6 << 24) + min(c, 160);
        minValue = 0;
        divisor = 1;
    }
    void Train(const vector<pair<double, double>> &dataset);
    int Predict(double key) const;

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;             // 4 Byte
    double divisor;            // 4 Byte (TODO: float)
    double minValue;           // 4 Byte (TODO: float)
    unsigned short Offset[10]; // 20 Byte (TODO:Offset[11])
    unsigned short Base[10];   // 20 Byte
};

#endif