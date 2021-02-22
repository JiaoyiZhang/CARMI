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

    int childLeft;          // 4 Byte
    float divisor;          // 4->8 Byte
    double minValue;        // 8 Byte
    unsigned int table0[5]; // 20 Byte
    unsigned int table1[5]; // 20 Byte
};

#endif